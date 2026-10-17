import fs from 'fs';
import { createHost } from '../../../std/internal/runtime/host.mjs';
const f = process.argv[2];
let out = []; let mem;
const dec = new TextDecoder();
const env = { print_i64: x => out.push(String(x)), print_u64: x => out.push(String(BigInt.asUintN(64, x))), print_f64: x => out.push(String(x)),
  print_bool: b => out.push(b ? 'true' : 'false'), print_str: (p, n) => out.push(dec.decode(new Uint8Array(mem.buffer, Number(p), Number(n)))) };
let text = '';
const h = createHost({ write: (fd, b) => { if (fd === 1) { text += dec.decode(b); return b.length; } return fs.writeSync(fd, b); } });
let trapped = false;
try { const i = h.instantiate(fs.readFileSync(f), { env }); mem = i.exports.memory; (i.exports.run || i.exports.main)(); }
catch (e) { trapped = true; out.push('TRAP ' + e.message); }
h.close();
console.log([...out, ...text.split('\n').filter(x => x)].join('\n'));
