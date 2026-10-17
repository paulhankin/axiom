#!/usr/bin/env python3
"""Generates std/audio/testdata: Opus streams from libopus (through ffmpeg)
in each of its modes, with libopus's own decoding (through ctypes) as
16-bit PCM to compare with; AAC streams (ADTS and MP4) from FFmpeg's
encoder in each of its profiles and tools, and HE-AAC (v1 and v2) streams
from the Fraunhofer encoder (fdk-aac's aac-enc, built from
https://github.com/mstorsjo/fdk-aac; its path in FDK_AAC_ENC), with
FFmpeg's decoding.

    cmd/axiom/internal/tests/tools/gen_audio_tests.py [opus|aac ...]
"""
import ctypes
import math
import os
import random
import struct
import subprocess
import sys
import wave

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..', '..', 'std', 'audio', 'testdata')


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


def aac():
    d = os.path.join(root, 'aac')
    os.makedirs(d, exist_ok=True)
    tmp = os.path.join(d, 'src.wav')
    cases = []

    def enc(name, ext, frames, rate, params):
        write_wav(tmp, frames, rate)
        out = os.path.join(d, name + '.' + ext)
        subprocess.run(['ffmpeg', '-v', 'error', '-y', '-i', tmp, '-c:a', 'aac'] + params + [out], check=True)
        raw = subprocess.run(['ffmpeg', '-v', 'error', '-i', out, '-f', 'f32le', '-'], check=True,
                             capture_output=True).stdout
        pcm = struct.unpack('<%df' % (len(raw) // 4), raw)
        ch = len(frames[0])
        with open(os.path.join(d, name + '.pcm'), 'wb') as f:
            f.write(b''.join(struct.pack('<h', max(-32768, min(32767, int(math.floor(v * 32768 + 0.5))))) for v in pcm))
        # FFmpeg trims an MP4's edit list at the start but not at the end;
        # the stream holds exactly the input
        want = len(frames) if ext == 'm4a' else len(pcm) // ch
        cases.append('%s %s %d %d %d %d' % (name, ext, ch, len(pcm) // ch, rate, want))

    # AAC-LC: sampling rates, mono and stereo, M/S, intensity stereo,
    # noise substitution, TNS, short windows (the clicks)
    enc('lc_stereo', 'm4a', signal(0.5, 2, 44100), 44100, ['-b:a', '128k'])
    enc('lc_low', 'm4a', signal(0.5, 2, 48000, seed=2), 48000, ['-b:a', '48k'])
    enc('lc_mono', 'aac', signal(0.5, 1, 22050, seed=4), 22050, ['-b:a', '40k'])
    enc('lc_8k', 'aac', speech(0.5, 1, 8000), 8000, ['-b:a', '16k'])
    enc('lc_96k', 'm4a', signal(0.25, 2, 96000, seed=6), 96000, ['-b:a', '192k'])
    enc('lc_12k', 'aac', speech(0.5, 2, 12000), 12000, ['-b:a', '24k'])
    enc('lc_fast', 'aac', signal(0.5, 2, 32000, seed=7), 32000, ['-b:a', '64k', '-aac_coder', 'fast'])
    enc('lc_pce', 'aac', signal(0.3, 2, 24000, seed=8), 24000, ['-b:a', '48k', '-aac_pce', '1'])
    # 5.1 (a channel configuration with LFE)
    enc('surround', 'm4a', signal(0.3, 6, 48000, seed=5), 48000, ['-b:a', '256k'])
    # AAC Main (prediction) and AAC LTP
    enc('main', 'm4a', signal(0.5, 2, 44100, seed=9), 44100, ['-profile:a', 'aac_main', '-aac_pred', '1', '-strict', '-2', '-b:a', '96k'])
    enc('ltp', 'aac', signal(0.5, 2, 44100, seed=10), 44100, ['-profile:a', 'aac_ltp', '-aac_ltp', '1', '-strict', '-2', '-b:a', '96k'])
    # HE-AAC: spectral band replication, parametric stereo, in ADTS and in
    # MP4 with implicit and explicit signalling
    enc_bin = os.environ.get('FDK_AAC_ENC')
    if enc_bin:
        def fdk(name, frames, rate, aot, bitrate, explicit=None):
            write_wav(tmp, frames, rate)
            adts = os.path.join(d, name + '.aac')
            env = dict(os.environ, LD_LIBRARY_PATH=os.path.dirname(enc_bin))
            subprocess.run([enc_bin, '-t', str(aot), '-r', str(bitrate), tmp, adts], check=True, env=env,
                           capture_output=True)
            out = adts
            ext = 'aac'
            if explicit is not None:
                ext = 'm4a'
                out = os.path.join(d, name + '.m4a')
                subprocess.run(['ffmpeg', '-v', 'error', '-y', '-i', adts, '-c', 'copy', out], check=True)
                os.remove(adts)
                if explicit:
                    explicit_sbr(out, aot, len(frames[0]), rate)
            raw = subprocess.run(['ffmpeg', '-v', 'error', '-i', out, '-f', 'f32le', '-'], check=True,
                                 capture_output=True).stdout
            pcm = struct.unpack('<%df' % (len(raw) // 4), raw)
            ch = 2  # FFmpeg decodes mono HE-AAC as stereo (parametric stereo may follow)
            with open(os.path.join(d, name + '.pcm'), 'wb') as f:
                f.write(b''.join(struct.pack('<h', max(-32768, min(32767, int(math.floor(v * 32768 + 0.5)))))
                                 for v in pcm))
            cases.append('%s %s %d %d %d %d' % (name, ext, ch, len(pcm) // ch, rate, len(pcm) // ch))

        fdk('he', signal(0.5, 2, 44100, seed=11), 44100, 5, 48000)
        fdk('he_mono', signal(0.5, 1, 48000, seed=12), 48000, 5, 32000)
        fdk('he_v2', signal(0.5, 2, 44100, seed=13), 44100, 29, 32000)
        fdk('he_implicit', signal(0.4, 2, 32000, seed=14), 32000, 5, 40000, explicit=False)
        fdk('he_explicit', signal(0.4, 2, 44100, seed=15), 44100, 5, 48000, explicit=True)
        fdk('he_v2_explicit', signal(0.4, 2, 48000, seed=16), 48000, 29, 32000, explicit=True)
    os.remove(tmp)
    with open(os.path.join(d, 'cases.txt'), 'w') as f:
        f.write('\n'.join(cases) + '\n')
    print(len(cases), 'aac cases')


RATES = [96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350]


def explicit_sbr(path, aot, channels, rate):
    """Rewrites an MP4's AudioSpecificConfig to signal SBR (and PS)
    explicitly (object type 5 or 29) instead of implicitly."""
    def asc():
        core = RATES.index(rate // 2)
        chan = 1 if aot == 29 else channels
        bits = '{:05b}{:04b}{:04b}{:04b}{:05b}000'.format(aot, core, chan, RATES.index(rate), 2)
        bits += '0' * (-len(bits) % 8)
        return bytes(int(bits[i:i + 8], 2) for i in range(0, len(bits), 8))

    def desc(tag, payload):
        n = len(payload)
        return bytes([tag, 0x80 | (n >> 21) & 0x7f, 0x80 | (n >> 14) & 0x7f, 0x80 | (n >> 7) & 0x7f, n & 0x7f]) + payload

    def rebuild(data, o, end):
        out = b''
        while o < end:
            size, typ = struct.unpack('>I4s', data[o:o + 8])
            body = data[o + 8:o + size]
            if typ in (b'moov', b'trak', b'mdia', b'minf', b'stbl'):
                body = rebuild(data, o + 8, o + size)
            elif typ == b'stsd':
                entry = body[8:]
                esize, etyp = struct.unpack('>I4s', entry[:8])
                fields = entry[8:36]
                kids = entry[36:esize]
                k = 0
                new = b''
                while k < len(kids):
                    ksize, ktyp = struct.unpack('>I4s', kids[k:k + 8])
                    if ktyp == b'esds':
                        kb = kids[k + 8:k + ksize]
                        es_hdr = kb[9:12]
                        dc_fixed = kb[17:30]
                        esd = desc(3, es_hdr + desc(4, dc_fixed + desc(5, asc())) + desc(6, b'\x02'))
                        new += struct.pack('>I4s', 12 + len(esd), b'esds') + kb[:4] + esd
                    else:
                        new += kids[k:k + ksize]
                    k += ksize
                body = body[:8] + struct.pack('>I4s', 36 + len(new), etyp) + fields + new
            out += struct.pack('>I4s', 8 + len(body), typ) + body
            o += size
        return out

    data = open(path, 'rb').read()
    assert data.find(b'moov') > data.find(b'mdat')  # chunk offsets stay valid
    open(path, 'wb').write(rebuild(data, 0, len(data)))


for what in sys.argv[1:] or ['opus']:
    globals()[what]()
