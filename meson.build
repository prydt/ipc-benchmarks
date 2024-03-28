project('ipc-benchmarks', 'c', default_options: ['debug=true', 'warning_level=1'])

cpu_configs = [
    ['same', '-DIPC_CPU_SAME'],
    ['hyperthread', '-DIPC_CPU_HYPERTHREAD'], 
    ['different', '-DIPC_CPU_DIFFERENT']
]
program_configs = [
    ['condvar', '-DIPC_CONDVAR_BENCH'],
    ['futex', '-DIPC_FUTEX_BENCH'],
    ['atomic_yield', '-DIPC_ATOMIC_YIELD_BENCH'],
    ['pipe', '-DIPC_PIPE_BENCH'],
    ['socket', '-DIPC_SOCKET_BENCH']
]

src = [
    'ipc_atomic_yield.c',
    'ipc_condvar.c',
    'ipc_futex.c',
    'ipc_pipe.c',
    'ipc_socket.c',
    'runner.c',
    'utils.c'
]

thread_dep = dependency('threads')

foreach c : cpu_configs
    foreach p : program_configs
        program_name = p[0] + '_' + c[0]
        executable(program_name, src, c_args: [c[1], p[1]],  dependencies: thread_dep)
    endforeach
endforeach