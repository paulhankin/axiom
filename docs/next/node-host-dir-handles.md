## Standard library

- Under the node host, `fs.remove_all` and `fs.walk` no longer follow a symbolic
  link swapped in for a directory while they run: directory handles pin the
  directory they opened (through `/proc/self/fd` on Linux; by a device and inode
  check elsewhere).
