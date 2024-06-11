# %%
import matplotlib.pyplot as plt
import numpy as np
import subprocess
from statistics import mean, stdev
from collections import defaultdict


# %%
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

# %%
REPEAT_TRIALS = 10


# Returns ping-pong time divided by 2
def run_command(name):
    return float(
        subprocess.run([name], capture_output=True).stdout.decode("utf-8").strip()
    ) / (num_iters * 2)


def run_benchmark(name, dir="./"):
    return [run_command(dir + name) for i in range(REPEAT_TRIALS)]


num_iters = 1000 * 100
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
            runs = run_benchmark(name, "./builddir/")
            benchmark_means[topology].append(mean(runs))
            benchmark_stdevs[topology].append(stdev(runs))

print(benchmark_means)
print(benchmark_stdevs)

# %%
# fig, ax = plt.subplots(layout="constrained")
fig, ax = plt.subplots()

# graph settings
width = 0.25
multiplier = 0
x = np.arange(len(ipc_types))

for attribute, measurement in benchmark_means.items():
    offset = width * multiplier
    rects = ax.bar(x + offset, measurement, width, label=attribute)
    # ax.bar_label(rects, padding=3)
    # ax.bar_label(rects)#, padding=3)
    err = ax.errorbar(
        x=x + offset, y=measurement, yerr=benchmark_stdevs[attribute], fmt="_r"
    )
    multiplier += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel("Time (ns)")
# ax.set_title("IPC Type")
ax.set_xlabel("IPC Type")
plt.xticks(rotation=50)
ax.set_xticks(x + width, ipc_types)
ax.legend()
# ax.set_ylim(0, 250)

plt.tight_layout()
plt.savefig("benchmark_graph.png")
plt.show()

# %%
