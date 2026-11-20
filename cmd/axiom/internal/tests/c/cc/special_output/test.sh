# An output that is not a regular file (/dev/null, which configure probes and
# makefiles link to; a FIFO here) is written, and neither replaced nor given
# an executable's permissions.
. "$CC_LIB"
command -v mkfifo > /dev/null || { echo "(no mkfifo: skipped)"; exit 0; }
for args in "main.c" "-c main.c" "-shared main.c"; do
    rm -f out.fifo
    mkfifo out.fifo || fail "mkfifo"
    chmod 644 out.fifo
    cat out.fifo > got.bin &
    reader=$!
    run $CC $OPT $args -o out.fifo
    wait $reader
    [ -p out.fifo ] || fail "$args -o out.fifo: no longer a FIFO"
    mode=$(stat -c %a out.fifo 2> /dev/null || stat -f %Lp out.fifo)
    [ "$mode" = 644 ] || fail "$args -o out.fifo: its permissions are now $mode"
    [ -s got.bin ] || fail "$args -o out.fifo: nothing written"
done
exit 0
