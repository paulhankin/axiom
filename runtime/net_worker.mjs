// runtime/net_worker.mjs - owns node's (asynchronous) sockets on behalf of
// the (synchronous) wasm threads; see NetClient in host.mjs.
//
// One worker serves every thread of a program, so socket handles are shared
// between threads. Each request carries the caller's own shared buffers:
// ctrl[0] is set to 1 (and notified) when the request completes, with its
// result in ctrl[1]; read data goes into `data`. Threads reach the worker
// through MessagePorts sent with a 'port' message.
import { parentPort } from 'worker_threads';
import net from 'net';

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

function addSocket(sock) {
  const st = { kind: 'sock', sock, chunks: [], ended: false, failed: false, waiters: [], timeout: 0 };
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
    } else if (st) {
      st.sock.end();
    }
    reply(req, 0);
  },
};

function serve(port) {
  port.on('message', (m) => {
    if (m.op === 'port') serve(m.port);
    else ops[m.op](m, m.args || {}, m.payload);
  });
}

serve(parentPort);
