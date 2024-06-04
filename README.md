# ipc-benchmarks

Benchmarks implemented
 - [x] shared memory + pthread condvars
 - [x] shared memory + futexes
 - [x] shared memory + C11 atomics (yield)
 - [x] shared memory + C11 atomics (spin)
 - [x] shared memory + POSIX Semaphores
 - [x] Unix Sockets
 - [x] Pipes
 - [x] POSIX Message Queues
 - [x] System V Message Queues
 - [x] System V Semaphores


# Build
You will need Meson and Ninja installed. Python 3 is required to run the benchmark scripts.
```bash
meson setup builddir && cd builddir
meson compile
```