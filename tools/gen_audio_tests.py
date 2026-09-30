#!/usr/bin/env python3
"""Generates std/audio/testdata: Opus streams from libopus (through ffmpeg)
in each of its modes, with libopus's own decoding (through ctypes) as
16-bit PCM to compare with.

    tools/gen_audio_tests.py [opus ...]
"""
import ctypes
import math
import os
import random
import struct
import subprocess
import sys
import wave

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'std', 'audio', 'testdata')


def signal(seconds, channels, rate=48000, seed=0):
    """Music-like test audio: chords with vibrato, a chirp, noise bursts,
    clicks and a short silence, different in each channel."""
    r = random.Random(seed)
    n = int(seconds * rate)
    out = []
    notes = [220.0, 277.18, 329.63, 440.0, 554.37]
    for i in range(n):
        tt = i / rate
        frame = []
        for c in range(channels):
            v = 0.0
            for k, f in enumerate(notes[:3 + c % 3]):
                v += 0.12 * math.sin(2 * math.pi * f * (1 + 0.003 * math.sin(2 * math.pi * 5 * tt)) * tt + k + c)
            v += 0.15 * math.sin(2 * math.pi * (300 + 4000 * tt / seconds) * tt * (1 + c * 0.1))
            if int(tt * 7) % 3 == 1:
                v += 0.2 * (r.random() - 0.5)
            if i % 9600 < 30:
                v += 0.6 * (1 if (i // 30) % 2 else -1)
            if 0.55 * seconds < tt < 0.62 * seconds:
                v *= 0.0
            frame.append(max(-1.0, min(1.0, v * (0.8 + 0.2 * c / max(channels, 1)))))
        out.append(frame)
    return out


def speech(seconds, channels, rate=48000, seed=1):
    """Speech-like audio: voiced segments with a varying pitch and formants,
    unvoiced noise, pauses."""
    r = random.Random(seed)
    n = int(seconds * rate)
    out = []
    phase = 0.0
    for i in range(n):
        tt = i / rate
        seg = int(tt * 8)
        voiced = seg % 4 != 3
        f0 = 110 + 40 * math.sin(2 * math.pi * 1.3 * tt) + 20 * (seg % 3)
        phase += 2 * math.pi * f0 / rate
        v = 0.0
        if voiced:
            for h in range(1, 25):
                amp = 1.0 / h
                for fm in (700 + 200 * (seg % 3), 1200 + 300 * (seg % 2), 2500):
                    d = (h * f0 - fm) / 150
                    amp += 0.6 * math.exp(-d * d) / h
                v += 0.08 * amp * math.sin(h * phase)
        else:
            v = 0.15 * (r.random() - 0.5)
        if seg % 8 == 7:
            v *= 0.02
        out.append([v * (1 - 0.3 * c) for c in range(channels)])
    return out


def write_wav(path, frames, rate=48000):
    ch = len(frames[0])
    with wave.open(path, 'wb') as w:
        w.setnchannels(ch)
        w.setsampwidth(2)
        w.setframerate(rate)
        w.writeframes(b''.join(struct.pack('<%dh' % ch, *[int(round(v * 32767)) for v in f]) for f in frames))


def ogg_packets(data):
    """The packets of an Ogg stream, with their pages' granule positions
    (on the last packet ending on a page)."""
    packets = []
    cur = b''
    o = 0
    while o < len(data):
        assert data[o:o + 4] == b'OggS'
        granule = struct.unpack('<q', data[o + 6:o + 14])[0]
        nseg = data[o + 26]
        lacing = data[o + 27:o + 27 + nseg]
        p = o + 27 + nseg
        ends = []
        for lv in lacing:
            cur += data[p:p + lv]
            p += lv
            if lv < 255:
                packets.append([cur, -1])
                ends.append(len(packets) - 1)
                cur = b''
        if ends:
            packets[ends[-1]][1] = granule
        o = p
    return packets


opus_lib = None


def libopus_decode(data):
    """libopus's decoding of an Ogg Opus stream (as floats), trimmed by the
    pre-skip and the last granule position."""
    global opus_lib
    if opus_lib is None:
        opus_lib = ctypes.CDLL('libopus.so.0')
    pk = ogg_packets(data)
    head = pk[0][0]
    assert head[:8] == b'OpusHead'
    channels = head[9]
    pre_skip = struct.unpack('<H', head[10:12])[0]
    gain = struct.unpack('<h', head[16:18])[0]
    family = head[18]
    err = ctypes.c_int()
    if family == 0:
        streams, coupled, mapping = 1, channels - 1, bytes(range(channels))
    else:
        streams, coupled = head[19], head[20]
        mapping = head[21:21 + channels]
    dec = opus_lib.opus_multistream_decoder_create(48000, channels, streams, coupled,
                                                   (ctypes.c_ubyte * channels)(*mapping), ctypes.byref(err))
    assert err.value == 0
    if gain:
        opus_lib.opus_multistream_decoder_ctl(dec, 4034, ctypes.c_int(gain))  # OPUS_SET_GAIN
    buf = (ctypes.c_float * (5760 * channels))()
    out = []
    last_granule = 0
    for p, g in pk[2:]:
        n = opus_lib.opus_multistream_decode_float(dec, p, len(p), buf, 5760, 0)
        assert n > 0, n
        out.extend(buf[:n * channels])
        if g >= 0:
            last_granule = g
    total = last_granule - pre_skip
    out = out[pre_skip * channels:]
    out = out[:total * channels]
    return channels, out


def opus():
    d = os.path.join(root, 'opus')
    os.makedirs(d, exist_ok=True)
    tmp = os.path.join(d, 'src.wav')
    cases = []

    def enc(name, frames, params):
        write_wav(tmp, frames)
        out = os.path.join(d, name + '.opus')
        subprocess.run(['ffmpeg', '-v', 'error', '-y', '-i', tmp, '-c:a', 'libopus'] + params + [out], check=True)
        data = open(out, 'rb').read()
        ch, pcm = libopus_decode(data)
        with open(os.path.join(d, name + '.pcm'), 'wb') as f:
            f.write(b''.join(struct.pack('<h', max(-32768, min(32767, int(math.floor(v * 32768 + 0.5))))) for v in pcm))
        cases.append('%s %d %d' % (name, ch, len(pcm) // ch))

    music2 = signal(0.5, 2)
    music1 = signal(0.5, 1, seed=3)
    talk1 = speech(0.6, 1)
    talk2 = speech(0.6, 2)
    # CELT only (low delay): frame sizes, rates, mono and stereo, CBR
    enc('celt_20ms', music2, ['-application', 'lowdelay', '-b:a', '128k'])
    enc('celt_2_5ms', music2, ['-application', 'lowdelay', '-b:a', '96k', '-frame_duration', '2.5'])
    enc('celt_5ms_mono', music1, ['-application', 'lowdelay', '-b:a', '48k', '-frame_duration', '5'])
    enc('celt_10ms', music2, ['-application', 'lowdelay', '-b:a', '64k', '-frame_duration', '10'])
    enc('celt_low', music2, ['-application', 'lowdelay', '-b:a', '16k'])
    enc('celt_cbr', music1, ['-application', 'lowdelay', '-b:a', '32k', '-vbr', 'off'])
    enc('celt_60ms', music2, ['-application', 'lowdelay', '-b:a', '64k', '-frame_duration', '60'])
    # SILK only: narrow, medium and wide band, long frames, stereo, FEC
    enc('silk_nb', talk1, ['-application', 'voip', '-b:a', '8k', '-cutoff', '4000'])
    enc('silk_mb', talk1, ['-application', 'voip', '-b:a', '10k', '-cutoff', '6000'])
    enc('silk_wb', talk1, ['-application', 'voip', '-b:a', '16k', '-cutoff', '8000'])
    enc('silk_40ms', talk1, ['-application', 'voip', '-b:a', '12k', '-frame_duration', '40'])
    enc('silk_60ms', talk1, ['-application', 'voip', '-b:a', '12k', '-frame_duration', '60'])
    enc('silk_stereo', talk2, ['-application', 'voip', '-b:a', '24k', '-cutoff', '8000'])
    enc('silk_fec', talk1, ['-application', 'voip', '-b:a', '20k', '-fec', '1', '-packet_loss', '20'])
    # hybrid, and switching between modes
    enc('hybrid_swb', talk1, ['-application', 'voip', '-b:a', '28k', '-cutoff', '12000'])
    enc('hybrid_fb', talk2, ['-application', 'audio', '-b:a', '40k'])
    enc('switching', talk2[:14400] + music2[:14400] + talk2[14400:], ['-application', 'audio', '-b:a', '48k'])
    # more than two channels (mapping family 1)
    enc('surround', signal(0.3, 6, seed=5), ['-b:a', '192k', '-mapping_family', '1'])
    os.remove(tmp)
    with open(os.path.join(d, 'cases.txt'), 'w') as f:
        f.write('\n'.join(cases) + '\n')
    print(len(cases), 'opus cases')


for what in sys.argv[1:] or ['opus']:
    globals()[what]()
