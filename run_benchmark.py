#!/bin/env python3

import subprocess
from statistics import mean, stdev
from collections import defaultdict

REPEAT_TRIALS = 5


# Returns ping-pong time divided by 2
def run_command(name):
    return float(
        subprocess.run(["./" + name], capture_output=True)
        .stdout.decode("utf-8")
        .strip()
    ) / (num_iters * 2)


def run_benchmark(name):
    # return (float(median([run_command(name) for i in range(100)])) / num_iters) / 2
    return [run_command(name) for i in range(REPEAT_TRIALS)]


num_iters = 1000 * 100
ipc_types = [
    "condvar",
    "futex",
    "atomic_yield",
    "atomic_spin",
    "socket",
    "pipe",
    "sv_mq",
    "sv_sema",
    "posix_mq",
    "posix_sema",
]

benchmark_means = defaultdict(list)
benchmark_stdevs = defaultdict(list)

for ipc in ipc_types:
    for topology in ["same", "hyperthread", "different"]:
        name = ipc + "_" + topology
        print(name)
        if name == "atomic_spin_same":
            benchmark_means[topology].append(0)
            benchmark_stdevs[topology].append(0)
        else:
            runs = run_benchmark(name)
            benchmark_means[topology].append(mean(runs))
            benchmark_stdevs[topology].append(stdev(runs))


print("benchmark_means:")
print(benchmark_means)
print("benchmark_stdev:")
print(benchmark_stdevs)