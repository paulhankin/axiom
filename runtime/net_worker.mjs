// runtime/net_worker.mjs - owns node's (asynchronous) sockets on behalf of
// the (synchronous) wasm thread; see NetClient in host.mjs.
import { parentPort, workerData } from 'worker_threads';
import net from 'net';

const ctrl = new Int32Array(workerData.ctrl);
const data = new Uint8Array(workerData.data);
const handles = new Map();
let nextHandle = 3;

function done(result) {
  Atomics.store(ctrl, 1, result);
  Atomics.store(ctrl, 0, 1);
  Atomics.notify(ctrl, 0);
}

function errCode(e) {
  switch (e && e.code) {
    case 'ECONNREFUSED': return -2;
    case 'EADDRINUSE': return -3;
    case 'ENOTFOUND': case 'EAI_AGAIN': case 'ERR_INVALID_IP_ADDRESS': case 'ERR_INVALID_ADDRESS': return -4;
    default: return -1;
  }
}

function wake(st) {
  const w = st.waiter;
  st.waiter = null;
  if (w) w();
}

function addSocket(sock) {
  const st = { kind: 'sock', sock, chunks: [], ended: false, failed: false, waiter: null };
  sock.on('data', (b) => { st.chunks.push(b); wake(st); });
  sock.on('end', () => { st.ended = true; wake(st); });
  sock.on('close', () => { st.ended = true; wake(st); });
  sock.on('error', () => { st.failed = true; st.ended = true; wake(st); });
  const h = nextHandle++;
  handles.set(h, st);
  return h;
}

const ops = {
  connect({ host, port }) {
    const sock = net.connect({ host: host || '127.0.0.1', port });
    const onError = (e) => done(errCode(e));
    sock.once('error', onError);
    sock.once('connect', () => { sock.off('error', onError); done(addSocket(sock)); });
  },
  listen({ host, port }) {
    const srv = net.createServer();
    const st = { kind: 'listener', srv, queue: [], waiter: null };
    srv.on('connection', (s) => { st.queue.push(s); wake(st); });
    const onError = (e) => done(errCode(e));
    srv.once('error', onError);
    srv.listen(port, host || undefined, () => {
      srv.off('error', onError);
      const h = nextHandle++;
      handles.set(h, st);
      done(h);
    });
  },
  accept({ h }) {
    const st = handles.get(h);
    if (!st || st.kind !== 'listener') return done(-1);
    const attempt = () => {
      if (st.queue.length) done(addSocket(st.queue.shift()));
      else st.waiter = attempt;
    };
    attempt();
  },
  local_port({ h }) {
    const st = handles.get(h);
    if (!st) return done(-1);
    done(st.kind === 'listener' ? st.srv.address().port : st.sock.localPort);
  },
  read({ h, n }) {
    const st = handles.get(h);
    if (!st || st.kind !== 'sock') return done(-1);
    const attempt = () => {
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
        done(got);
      } else if (st.ended) {
        done(st.failed ? -1 : 0);
      } else {
        st.waiter = attempt;
      }
    };
    attempt();
  },
  write({ h }, payload) {
    const st = handles.get(h);
    if (!st || st.kind !== 'sock' || st.failed) return done(-1);
    st.sock.write(payload, (err) => done(err ? -1 : payload.length));
  },
  close({ h }) {
    const st = handles.get(h);
    handles.delete(h);
    if (st && st.kind === 'listener') st.srv.close();
    else if (st) st.sock.end();
    done(0);
  },
};

parentPort.on('message', (m) => ops[m.op](m.args || {}, m.payload));
