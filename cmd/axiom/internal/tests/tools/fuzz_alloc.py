#!/usr/bin/env python3
"""Randomized multithreaded fuzzing of the global allocator (std/mem).

    cmd/axiom/internal/tests/tools/fuzz_alloc.py [--runs N | --minutes M] [--jobs J] [--seed K] [--timeout S] [lane ...]

Builds cmd/axiom/internal/tests/fuzz/alloc.ax once per lane, then runs it with
random seeds, the lanes in turn. Each seed picks a configuration (threads,
sizes, how often objects are reallocated, aligned, zeroed, passed to another
thread, freed in bursts; sleeps long enough for free memory to be given
back), and the program checks every object's contents and that the
allocator's counters come back to where they started. A nonzero exit, a
trap, a signal or a hang is a failure: its seed and output are saved under
build/fuzz/failures/alloc/<lane>-<seed>.txt, with the command that repeats it
(failures with threads need not repeat every time; run it a few times).
`alloc <seed> <threads> <mask>` narrows one down (see alloc.ax).

Lanes: o2 and o0 (native, the compiler), asan (native, --sanitize=address),
llvm (native, the bootstrap compiler, if it is built) and wasm (wasm32 under
node). All by default. --runs N runs N seeds in all (200 by default);
--minutes M runs for about M minutes instead. --seed 0 picks a seed from the
clock and prints it. Each run starts up to 13 threads, so --jobs defaults to
half the processors.
"""
import argparse
import concurrent.futures
import os
import random
import subprocess
import sys
import threading
import time

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..', '..'))
OUT = os.path.join(ROOT, 'build', 'fuzz')
SRC = os.path.join(ROOT, 'cmd', 'axiom', 'internal', 'tests', 'fuzz', 'alloc.ax')
HOST = os.path.join(ROOT, 'std', 'internal', 'runtime', 'host.mjs')
LANES = ['o2', 'asan', 'o0', 'llvm', 'wasm']


def build(lane, compiler, bootstrap):
    exe = os.path.join(OUT, 'alloc-' + lane)
    flags = {
        'o2': [compiler, '--target=native', '-O2'],
        'o0': [compiler, '--target=native', '-O0'],
        'asan': [compiler, '--target=native', '-O2', '--sanitize=address'],
        'llvm': [bootstrap, '--target=native', '-O2'],
        'wasm': [compiler, '--target=wasm32', '-O2'],
    }[lane]
    if lane == 'wasm':
        exe += '.wasm'
    subprocess.run([flags[0], 'build'] + flags[1:] + ['-o', exe, SRC], check=True)
    if lane == 'wasm':
        return lambda seed: ['node', HOST, exe, '--', str(seed)]
    return lambda seed: [exe, str(seed)]


def run_one(lane, cmd, seed, timeout):
    """None if the run passed, else what to save."""
    try:
        p = subprocess.run(cmd(seed), stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    except subprocess.TimeoutExpired:
        return 'timeout after %g s' % timeout
    if p.returncode == 0:
        return None
    code = p.returncode if p.returncode >= 0 else 'signal %d' % -p.returncode
    return 'exit %s\n%s' % (code, p.stdout.decode('utf-8', 'replace')[-8000:])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--runs', type=int, default=200)
    ap.add_argument('--minutes', type=float, default=0, help='a time budget, in place of --runs')
    ap.add_argument('--timeout', type=float, default=600.0)
    ap.add_argument('--jobs', type=int, default=max(1, (os.cpu_count() or 2) // 2))
    ap.add_argument('--seed', type=int, default=1)
    ap.add_argument('--compiler', default=os.environ.get('AXIOM', os.path.join(ROOT, 'cmd', 'axiom', 'build', 'axiom')))
    ap.add_argument('--bootstrap', default=os.path.join(ROOT, 'cmd', 'axiom', 'bootstrap', 'build', 'axiom'))
    ap.add_argument('lanes', nargs='*')
    a = ap.parse_args()
    lanes = a.lanes or [l for l in LANES if l != 'llvm' or os.access(a.bootstrap, os.X_OK)]
    for l in lanes:
        if l not in LANES:
            sys.exit('fuzz_alloc: no lane %r (%s)' % (l, ' '.join(LANES)))
    if a.seed == 0:
        a.seed = int(time.time()) % 1000000
        print('seed %d' % a.seed)
    os.makedirs(OUT, exist_ok=True)
    fails_dir = os.path.join(OUT, 'failures', 'alloc')
    cmds = {l: build(l, a.compiler, a.bootstrap) for l in lanes}
    rng = random.Random(a.seed)
    lock = threading.Lock()
    state = {'n': 0, 'runs': {l: 0 for l in lanes}, 'failed': []}
    deadline = time.time() + a.minutes * 60 if a.minutes else 0

    def next_job():
        with lock:
            if (deadline and time.time() >= deadline) or (not deadline and state['n'] >= a.runs):
                return None
            lane = lanes[state['n'] % len(lanes)]
            state['n'] += 1
            return lane, rng.getrandbits(48)

    def worker():
        while True:
            job = next_job()
            if job is None:
                return
            lane, seed = job
            bad = run_one(lane, cmds[lane], seed, a.timeout)
            with lock:
                state['runs'][lane] += 1
                if bad is not None:
                    os.makedirs(fails_dir, exist_ok=True)
                    path = os.path.join(fails_dir, '%s-%d.txt' % (lane, seed))
                    with open(path, 'w') as f:
                        f.write('repeat: %s\n%s\n' % (' '.join(cmds[lane](seed)), bad))
                    state['failed'].append(path)
                    print('FAIL %s seed %d: %s' % (lane, seed, bad.split('\n')[0]), flush=True)

    with concurrent.futures.ThreadPoolExecutor(max_workers=a.jobs) as ex:
        for f in [ex.submit(worker) for _ in range(a.jobs)]:
            f.result()
    for l in lanes:
        print('%-5s %6d runs' % (l, state['runs'][l]))
    print('%d failures' % len(state['failed']))
    for p in state['failed'][:20]:
        print('    ' + p)
    sys.exit(1 if state['failed'] else 0)


if __name__ == '__main__':
    main()
