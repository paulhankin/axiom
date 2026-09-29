// runtime/net_worker.mjs - owns node's (asynchronous) sockets on behalf of
// the (synchronous) wasm threads; see NetClient in host.mjs.
//
// One worker serves every thread of a program, so socket handles are shared
// between threads. Each request carries the caller's own shared buffers:
// ctrl[0] is set to 1 (and notified) when the request completes, with its
// result in ctrl[1]; read data goes into `data`. Threads reach the worker
// through MessagePorts sent with a 'port' message.
import { parentPort } from 'worker_threads';
import child_process from 'child_process';
import net from 'net';
import os from 'os';

const handles = new Map();
let nextHandle = 3;

const ERR = -1, REFUSED = -2, ADDR_IN_USE = -3, BAD_ADDRESS = -4, TIMED_OUT = -5;

function reply(req, result) {
  const ctrl = new Int32Array(req.ctrl);
  Atomics.store(ctrl, 1, result);
  Atomics.store(ctrl, 0, 1);
  Atomics.notify(ctrl, 0);
}

function errCode(e) {
  switch (e && e.code) {
    case 'ECONNREFUSED': return REFUSED;
    case 'EADDRINUSE': return ADDR_IN_USE;
    case 'ENOTFOUND': case 'EAI_AGAIN': case 'ERR_INVALID_IP_ADDRESS': case 'ERR_INVALID_ADDRESS': return BAD_ADDRESS;
    default: return ERR;
  }
}

// Runs the queued waiters that can now make progress.
function wake(st) {
  const ws = st.waiters;
  st.waiters = [];
  for (const w of ws) w();
}

function addSocket(sock, readOnly = false) {
  const st = { kind: 'sock', sock, chunks: [], ended: false, failed: false, waiters: [], timeout: 0, readOnly };
  sock.on('data', (b) => { st.chunks.push(b); wake(st); });
  sock.on('end', () => { st.ended = true; wake(st); });
  sock.on('close', () => { st.ended = true; wake(st); });
  sock.on('error', () => { st.failed = true; st.ended = true; wake(st); });
  const h = nextHandle++;
  handles.set(h, st);
  return h;
}

const ops = {
  connect(req, { host, port }) {
    const sock = net.connect({ host: host || '127.0.0.1', port });
    const onError = (e) => reply(req, errCode(e));
    sock.once('error', onError);
    sock.once('connect', () => { sock.off('error', onError); reply(req, addSocket(sock)); });
  },
  listen(req, { host, port }) {
    const srv = net.createServer();
    const st = { kind: 'listener', srv, queue: [], waiters: [], closed: false };
    srv.on('connection', (s) => { st.queue.push(s); wake(st); });
    const onError = (e) => reply(req, errCode(e));
    srv.once('error', onError);
    srv.listen(port, host || undefined, () => {
      srv.off('error', onError);
      const h = nextHandle++;
      handles.set(h, st);
      reply(req, h);
    });
  },
  accept(req, { h }) {
    const st = handles.get(h);
    if (!st || st.kind !== 'listener') return reply(req, ERR);
    const attempt = () => {
      if (st.closed) reply(req, ERR);
      else if (st.queue.length) reply(req, addSocket(st.queue.shift()));
      else st.waiters.push(attempt);
    };
    attempt();
  },
  local_port(req, { h }) {
    const st = handles.get(h);
    if (!st) return reply(req, ERR);
    reply(req, st.kind === 'listener' ? st.srv.address().port : st.sock.localPort);
  },
  read(req, { h, n }) {
    const st = handles.get(h);
    if (!st || st.kind !== 'sock') return reply(req, ERR);
    const data = new Uint8Array(req.data);
    let timer = null;
    let done = false;
    const attempt = () => {
      if (done) return;
      if (st.chunks.length) {
        let got = 0;
        while (got < n && st.chunks.length) {
          const c = st.chunks[0];
          const take = Math.min(c.length, n - got);
          data.set(c.subarray(0, take), got);
          got += take;
          if (take === c.length) st.chunks.shift();
          else st.chunks[0] = c.subarray(take);
        }
        done = true;
        if (timer) clearTimeout(timer);
        reply(req, got);
      } else if (st.ended) {
        done = true;
        if (timer) clearTimeout(timer);
        reply(req, st.failed ? ERR : 0);
      } else {
        st.waiters.push(attempt);
      }
    };
    attempt();
    if (!done && st.timeout > 0) {
      timer = setTimeout(() => {
        if (done) return;
        done = true;
        st.waiters = st.waiters.filter((w) => w !== attempt);
        reply(req, TIMED_OUT);
      }, st.timeout);
    }
  },
  write(req, { h }, payload) {
    const st = handles.get(h);
    if (!st || st.kind !== 'sock' || st.failed) return reply(req, ERR);
    st.sock.write(payload, (err) => reply(req, err ? ERR : payload.length));
  },
  timeout(req, { h, ms }) {
    const st = handles.get(h);
    if (!st || st.kind !== 'sock') return reply(req, ERR);
    st.timeout = ms;
    reply(req, 0);
  },
  close(req, { h }) {
    const st = handles.get(h);
    handles.delete(h);
    if (st && st.kind === 'listener') {
      st.closed = true;
      st.srv.close();
      wake(st);
    } else if (st && st.kind === 'sock') {
      if (st.readOnly) st.sock.destroy();
      else st.sock.end();
    }
    reply(req, 0);
  },
  // Child processes (see ph_proc_spawn): the handle, and pipe handles in
  // data as three i32s.
  spawn(req, { argv, env, cwd, modes }) {
    const out = new Int32Array(req.data, 0, 3);
    out.fill(-1);
    const stdio = modes.map((m, k) => (m === 1 ? 'pipe' : m === 2 ? 'ignore' : k));
    let child;
    try {
      child = child_process.spawn(argv[0], argv.slice(1), { env, cwd: cwd || undefined, stdio });
    } catch (e) {
      return reply(req, spawnErr(e));
    }
    const st = { kind: 'proc', child, exited: false, code: 0, signal: 0, waiters: [] };
    child.on('exit', (code, signal) => {
      st.exited = true;
      st.code = code === null ? 0 : code;
      st.signal = signal ? os.constants.signals[signal] || 0 : 0;
      wake(st);
    });
    child.once('error', (e) => { if (!st.started) reply(req, spawnErr(e)); });
    child.once('spawn', () => {
      st.started = true;
      const h = nextHandle++;
      handles.set(h, st);
      const pipes = [child.stdin, child.stdout, child.stderr];
      for (let k = 0; k < 3; k++) if (modes[k] === 1) out[k] = addSocket(pipes[k], k !== 0);
      reply(req, h);
    });
  },
  wait(req, { h, block }) {
    const st = handles.get(h);
    if (!st || st.kind !== 'proc') return reply(req, ERR);
    const out = new Int32Array(req.data, 0, 1);
    const attempt = () => {
      if (st.exited) {
        handles.delete(h);
        out[0] = st.signal || st.code;
        reply(req, st.signal ? 2 : 1);
      } else if (!block) {
        reply(req, 0);
      } else {
        st.waiters.push(attempt);
      }
    };
    attempt();
  },
  kill(req, { h, sig }) {
    const st = handles.get(h);
    if (!st || st.kind !== 'proc') return reply(req, ERR);
    reply(req, st.exited || st.child.kill(sig) ? 0 : ERR);
  },
};

function spawnErr(e) {
  switch (e && e.code) {
    case 'ENOENT': return -2;
    case 'EACCES': case 'EPERM': return -13;
    default: return ERR;
  }
}

function serve(port) {
  port.on('message', (m) => {
    if (m.op === 'port') serve(m.port);
    else ops[m.op](m, m.args || {}, m.payload);
  });
}

serve(parentPort);
