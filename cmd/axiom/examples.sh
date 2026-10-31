#!/bin/sh
# Builds every example in examples/ for the targets its header names, and runs
# the ones that can run here (docs/releasing.md: part of a release's build).
#
#   cmd/axiom/examples.sh [<compiler>]       (default cmd/axiom/build/axiom)
#
# The command-line examples run natively on Linux and macOS (and under node,
# for those built for WebAssembly), each with an input and a check of what it
# wrote; the WebAssembly modules without a main are loaded by node and their
# exports called. The examples with windows (Metal, Direct3D, games) are only
# built: they need a display. Output goes to build/examples/.
set -eu
cd "$(dirname "$0")/../.."

A=${1:-cmd/axiom/build/axiom}
[ -x "$A" ] || { echo "examples: no compiler at $A" >&2; exit 1; }
A=$(cd "$(dirname "$A")" && pwd)/$(basename "$A")
O=build/examples
rm -rf "$O"
mkdir -p "$O"
fails=0
fail() {
    echo "FAIL $*"
    fails=$((fails + 1))
}
# build <name> <source> <flags...>: builds $O/<name>
build() {
    n=$1
    src=$2
    shift 2
    if "$A" build "$src" "$@" -o "$O/$n" > "$O/$n.log" 2>&1; then
        echo "ok   build $n ($src $*)"
        return 0
    fi
    fail "build $n ($src $*):"
    sed 's/^/       /' "$O/$n.log" | head -5
    return 1
}
# check <name> <command...>: runs it, with its output in $O/<name>.out
check() {
    n=$1
    shift
    "$@" > "$O/$n.out" 2>&1
    st=$?
    if [ "$st" -eq 0 ]; then
        echo "ok   run   $n"
        return 0
    fi
    fail "run $n (status $st):"
    sed 's/^/       /' "$O/$n.out" | head -5
    return 1
}
# has <name> <text>: what <name> printed contains <text>
has() {
    grep -qF -- "$2" "$O/$1.out" || fail "$1 printed no '$2'"
}
# pdf <file>, png <file>: the file exists and starts as one
pdf() {
    head -c 5 "$1" 2>/dev/null | grep -q '%PDF-' || fail "$1 is not a PDF"
}
png() {
    head -c 4 "$1" 2>/dev/null | grep -q 'PNG' || fail "$1 is not a PNG"
}

native=""
case "$(uname -s)-$(uname -m)" in
    Linux-x86_64 | Linux-aarch64 | Darwin-arm64) native=--target=native ;;
esac

# Command-line programs, natively where this machine can run them.
if [ -n "$native" ]; then
    if build pi examples/pi/pi.ax $native -O2; then
        check pi "$O/pi" "$O/pi.pdf" && pdf "$O/pi.pdf"
    fi
    if build texmath examples/texmath/texmath.ax $native; then
        check texmath-text "$O/texmath" '\alpha^2 + \beta_1' && has texmath-text 'α² + β₁'
        check texmath-svg "$O/texmath" -o "$O/quadratic.svg" 'x = \frac{-b \pm \sqrt{b^2-4ac}}{2a}' &&
            { grep -q '<svg' "$O/quadratic.svg" || fail "quadratic.svg is not SVG"; }
        check texmath-pdf "$O/texmath" -o "$O/integral.pdf" '\int_0^\infty e^{-x^2}\,dx = \frac{\sqrt\pi}{2}' &&
            pdf "$O/integral.pdf"
    fi
    if build md2pdf examples/md2pdf/md2pdf.ax $native -O2; then
        check md2pdf "$O/md2pdf" README.md "$O/readme.pdf" && pdf "$O/readme.pdf"
    fi
    if build charts examples/charts/charts.ax $native -O2; then
        check charts "$O/charts" "$O/charts" && png "$O/charts.png" && pdf "$O/charts.pdf" &&
            { grep -q '<svg' "$O/charts.svg" || fail "charts.svg is not SVG"; }
    fi
    if build wordcount examples/wordcount/wordcount.ax $native -O2; then
        check wordcount sh -c "'$O/wordcount' < README.md" && has wordcount 'the'
    fi
    if build http_server examples/http_server/http_server.ax $native -O2; then
        # the notes API, from a client in node (on its port, 8080)
        "$O/http_server" > "$O/http_server.out" 2>&1 &
        pid=$!
        check http_server-client node -e '
            const base = "http://localhost:8080";
            const wait = (ms) => new Promise((r) => setTimeout(r, ms));
            (async () => {
                for (let i = 0; ; i++) {
                    try { await fetch(base + "/notes"); break; } catch (e) { if (i > 100) throw e; await wait(100); }
                }
                const add = await fetch(base + "/notes", { method: "POST", body: JSON.stringify({ text: "buy milk" }) });
                if (add.status >= 300) throw new Error("POST: " + add.status);
                const list = await (await fetch(base + "/notes")).text();
                if (!list.includes("buy milk")) throw new Error("GET: " + list);
                console.log(list);
            })().catch((e) => { console.error(String(e)); process.exit(1); });' || true
        kill "$pid" 2>/dev/null || true
        wait "$pid" 2>/dev/null || true
    fi
    # a library linked into a C program (Linux: macOS links its objects differently)
    if [ "$(uname -s)" = Linux ] && command -v cc > /dev/null &&
        build stats.o examples/c_interop/stats.ax $native --emit=obj --emit-header="$O/stats.h"; then
        if cc -I"$O" examples/c_interop/main.c "$O/stats.o" -o "$O/c_interop" > "$O/c_interop.log" 2>&1; then
            check c_interop "$O/c_interop" && has c_interop 'sorted 5 values'
        else
            fail "cc c_interop:"
            sed 's/^/       /' "$O/c_interop.log" | head -5
        fi
    fi
fi

# WebAssembly: programs under node's host, and modules without a main.
if build wordcount.wasm examples/wordcount/wordcount.ax -O2; then
    check wordcount-wasm sh -c "node std/internal/runtime/host.mjs '$O/wordcount.wasm' < README.md" &&
        has wordcount-wasm 'the'
fi
build generics.wasm examples/generics/generics.ax --target=wasm32-freestanding -O2 &&
    build shapes.wasm examples/shapes/shapes.ax --target=wasm32-freestanding &&
    build dot_product.wasm examples/dot_product/dot_product.ax --target=wasm32-freestanding -O3 &&
    check wasm-modules node -e '
        const fs = require("fs");
        // (the runtime writes a trap'"'"'s report with ax_write, and ends with ax_exit)
        let mem = null;
        const axiom = {
            ax_write: (fd, p, n) => { process.stderr.write(Buffer.from(mem.buffer, p, n)); return n; },
            ax_exit: (code) => { throw new Error("exit " + code); },
        };
        const load = (f, env) => {
            const e = new WebAssembly.Instance(new WebAssembly.Module(fs.readFileSync(f)), { env, axiom }).exports;
            mem = e.memory;
            return e;
        };
        const g = load(process.argv[1] + "/generics.wasm", {});
        const s = load(process.argv[1] + "/shapes.wasm", {});
        let logged = "";
        const d = load(process.argv[1] + "/dot_product.wasm", { console_log: (p, n) => {
            logged = Buffer.from(mem.buffer, p, n).toString(); } });
        const want = (what, got, expect) => { if (Math.abs(got - expect) > 1e-3) throw new Error(`${what}: ${got}, not ${expect}`); };
        want("generics demo()", g.demo(), 10);
        want("shapes shape_area(1, 2, 3)", s.shape_area(1, 2, 3), 6);
        want("shapes clz(1)", s.clz(1), 31);
        want("dot_product compute(1000)", d.compute(1000), 3000);
        if (logged !== "dot product computed") throw new Error("console_log: " + logged);
        console.log("ok");' "$O"

# Programs with windows, built for each system they support.
for t in aarch64-macos x86_64-windows; do
    os=${t#*-}
    gui=""
    [ "$os" = windows ] && gui=--subsystem=windows
    build "game-$os" examples/game/game.ax --target=$t -O2 || true
    build "swarm-$os" examples/swarm/swarm.ax --target=$t -O2 $gui || true
    build "trench-$os" examples/trench/trench.ax --target=$t -O2 $gui || true
    build "drumkin-$os" examples/drumkin --target=$t -O2 || true
    build "pdfview-$os" examples/pdfview --target=$t -O2 || true
done
build metal examples/metal/metal.ax --target=aarch64-macos -O2 || true
build pinball examples/pinball --target=aarch64-macos -O2 || true
build directx.exe examples/directx/directx.ax --target=x86_64-windows -O2 || true

if [ "$fails" -gt 0 ]; then
    echo "examples: $fails failed"
    exit 1
fi
echo "examples: all built and run"
