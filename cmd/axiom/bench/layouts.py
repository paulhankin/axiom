#!/usr/bin/env python3
"""Compares the code two compilers generate, on the C programs in cmd/axiom/bench/c/
(hot loops of different shapes: an interpreter's dispatch, a state machine, hash
probing, string scanning, sorting, matrix multiplies, bit manipulation, linked
structures, recursion, memory operations, struct copies, record coding, a
register machine) and the single-threaded Axiom ones beside it, measured so that
code placement does not decide the result: each program is built in several layouts
(AXIOM_X64_FPAD: every function shifted by that many bytes of padding, which moves
hot loops across cache line and decoder window boundaries; a kernel's time can
change by 10% from that alone), and the two compilers' builds of each layout are
run alternately.

    cmd/axiom/bench/layouts.py OLD NEW [--rounds N] [--filter S] [--pads 0,16,32,48] [-O2]
                                [--target T] [--cpu C] [--instructions]

OLD and NEW are compiler binaries. For each program it prints the median CPU time
(user + system) of each compiler's runs, the median of the paired ratios NEW/OLD
with their interquartile range, and at the end their geometric mean. The outputs
of every build must be the same. --cpu pins the runs to one processor (default 0;
-1: not pinned). --target is the programs' (default native). --instructions also
counts the instructions each build of the first layout executes, with valgrind's
cachegrind, which do not depend on the machine's load (valgrind does not run every
extension: --target=x86_64 for those counts). On a shared machine, prefer more
rounds and read the IQR.
"""
import argparse
import os
import shutil
import statistics
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..', '..', '..'))
AX = ['hashmap', 'matmul', 'sieve', 'sort', 'strings', 'alloc_tree']


def programs(flt):
    ps = [os.path.join(HERE, 'c', f) for f in sorted(os.listdir(os.path.join(HERE, 'c'))) if f.endswith('.c')]
    ps += [os.path.join(HERE, n + '.ax') for n in AX]
    return [p for p in ps if flt in os.path.basename(p)]


def cpu_time(cmd):
    p = subprocess.Popen(cmd, stdout=subprocess.DEVNULL)
    _, _, ru = os.wait4(p.pid, 0)
    return ru.ru_utime + ru.ru_stime


def instructions(exe):
    r = subprocess.run(['valgrind', '--tool=cachegrind', '--cache-sim=no', '--cachegrind-out-file=/dev/null', exe],
                       capture_output=True, text=True)
    for line in r.stderr.splitlines():
        if 'I   refs' in line or 'I refs' in line:
            return int(line.split()[-1].replace(',', ''))
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('old')
    ap.add_argument('new')
    ap.add_argument('--rounds', type=int, default=3)
    ap.add_argument('--filter', default='')
    ap.add_argument('--pads', default='0,16,32,48')
    ap.add_argument('--cpu', type=int, default=0)
    ap.add_argument('--target', default='native')
    ap.add_argument('--instructions', action='store_true')
    ap.add_argument('-O', dest='opt', default='2')
    a = ap.parse_args()
    pads = [int(x) for x in a.pads.split(',')]
    pin = [] if a.cpu < 0 else ['taskset', '-c', str(a.cpu)]
    tmp = tempfile.mkdtemp(prefix='axiom-layouts-')
    try:
        run(a, pads, pin, tmp)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


def run(a, pads, pin, tmp):
    ratios = []
    for prog in programs(a.filter):
        name = os.path.basename(prog)
        exes = []
        out0 = None
        for pad in pads:
            pair = []
            for tag, comp in (('old', a.old), ('new', a.new)):
                exe = os.path.join(tmp, '%s.%s.%d' % (name, tag, pad))
                env = dict(os.environ, AXIOM_X64_FPAD=str(pad))
                subprocess.run([comp, 'build', prog, '--std=' + os.path.join(ROOT, 'std'), '--target=' + a.target,
                                '-O' + a.opt, '-o', exe], check=True, env=env)
                out = subprocess.run([exe], capture_output=True, text=True).stdout
                if out0 is None:
                    out0 = out
                elif out != out0:
                    sys.exit('%s: different output (%s, padding %d)' % (name, tag, pad))
                pair.append(exe)
            exes.append(pair)
        to, tn, rs = [], [], []
        for _ in range(a.rounds):
            for old, new in exes:
                x = cpu_time(pin + [old])
                y = cpu_time(pin + [new])
                to.append(x)
                tn.append(y)
                rs.append(y / x)
        rs.sort()
        m = statistics.median(rs)
        ratios.append(m)
        line = '%-16s old %7.3f s  new %7.3f s  new/old %.3f  [IQR %.3f %.3f]' % (
            name, statistics.median(to), statistics.median(tn), m, rs[len(rs) // 4], rs[(3 * len(rs)) // 4])
        if a.instructions:
            io, inn = instructions(exes[0][0]), instructions(exes[0][1])
            line += '  instructions %+.2f%%' % (100.0 * (inn - io) / max(io, 1))
        print(line, flush=True)
    if ratios:
        g = 1.0
        for r in ratios:
            g *= r
        print('geometric mean new/old %.3f' % (g ** (1.0 / len(ratios))))


if __name__ == '__main__':
    main()
