# ipc-benchmarks

Benchmarks implemented
 - [x] shared memory + pthread condvars
 - [x] shared memory + futexes
 - [x] shared memory + C11 atomics (yield)
 - [x] shared memory + C11 atomics (spin)
 - [x] unix sockets
 - [x] pipes
 - [x] POSIX Message Queues
 - [ ] POSIX Semaphores
 - [x] System V Message Queues
 - [x] System V Semaphores


# Build
```bash
meson setup builddir && cd builddir
meson compile
```