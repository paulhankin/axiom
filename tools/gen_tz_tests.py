#!/usr/bin/env python3
"""Generates std/time/tz/testdata: TZif files of a few zones (copied from the
system's time-zone database, which is in the public domain), TZif files
holding only a POSIX TZ rule, and cases.txt with what Python's zoneinfo
says of them.

cases.txt lines:
  lookup FILE UNIX OFFSET DST ABBREV
  local FILE Y M D h m s FOLD UNIX
"""
import datetime
import os
import random
import shutil
import struct
import zoneinfo

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'std', 'time', 'tz', 'testdata')
os.makedirs(root, exist_ok=True)
ZONES = ['America/New_York', 'Europe/London', 'Europe/Dublin', 'Australia/Lord_Howe', 'Asia/Kolkata',
         'Pacific/Apia', 'America/Sao_Paulo', 'Africa/Casablanca', 'Asia/Tehran', 'Pacific/Chatham', 'UTC']
RULES = {
    'posix-us': 'EST5EDT,M3.2.0,M11.1.0',
    'posix-fixed': '<+0330>-3:30',
    'posix-south': 'AEST-10AEDT,M10.1.0,M4.1.0/3',
    'posix-israel': 'IST-2IDT,M3.4.4/26,M10.5.0',
    'posix-alldst': 'EST5EDT,0/0,J365/25',
    # (only J days: Python counts the zero-based n days from December 31)
    'posix-julian': 'XXX3YYY,J60/-1,J300/30:30:30',
    'posix-neg': '<-03>3<-02>,M3.5.0/-2,M10.5.0/-1',
}


def footer_only(rule):
    # a version 2 TZif file with one time type and the rule as its footer
    def header(ver):
        return b'TZif' + ver + b'\0' * 15 + struct.pack('>6l', 0, 0, 0, 0, 1, 4)
    block = struct.pack('>lBB', 0, 0, 0) + b'UTC\0'
    return header(b'2') + block + header(b'2') + block + b'\n' + rule.encode() + b'\n'


files = []
for z in ZONES:
    name = z.replace('/', '-') + '.tzif'
    shutil.copy(os.path.join('/usr/share/zoneinfo', z), os.path.join(root, name))
    files.append(name)
for n, r in RULES.items():
    with open(os.path.join(root, n + '.tzif'), 'wb') as f:
        f.write(footer_only(r))
    files.append(n + '.tzif')

rng = random.Random(8536)
lines = []
for name in files:
    with open(os.path.join(root, name), 'rb') as f:
        tz = zoneinfo.ZoneInfo.from_file(f, key=name)
    utc = datetime.timezone.utc
    ts = [rng.randint(-5000000000, 5000000000) for _ in range(16)]
    ts += [rng.randint(0, 2000000000) for _ in range(16)]
    # just around transitions
    prev = None
    for t in range(-2000000000, 4200000000, 3600 * 6):
        o = datetime.datetime.fromtimestamp(t, tz).utcoffset()
        if prev is not None and o != prev and len(ts) < 80 and rng.random() < 0.3:
            ts += [t - 3600 * 6 + k for k in range(0, 3600 * 6, 3600)]
        prev = o
    for t in ts:
        d = datetime.datetime.fromtimestamp(t, tz)
        lines.append('lookup %s %d %d %d %s' % (name, t, d.utcoffset().total_seconds(),
                                               1 if d.dst() else 0, d.tzname()))
        for fold in (0, 1):
            loc = d.replace(tzinfo=None)
            for delta in (0, 1800):
                l = loc + datetime.timedelta(seconds=delta)
                a = l.replace(tzinfo=tz, fold=fold)
                if a.year < 1 or a.year > 9998:
                    continue
                lines.append('local %s %d %d %d %d %d %d %d %d' % (name, l.year, l.month, l.day, l.hour, l.minute,
                                                                 l.second, fold, int(a.timestamp())))
with open(os.path.join(root, 'cases.txt'), 'w') as f:
    f.write('\n'.join(lines) + '\n')
print(len(files), len(lines))
