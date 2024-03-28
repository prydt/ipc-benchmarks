#!/bin/env python3
import csv
import subprocess
from statistics import median

num_iters = 1000 * 100
fieldnames = ['ipc', 'same CPU time (ns)', 'hyperthread core time (ns)', 'different core time (ns)']

ipc_types = ['condvar', 'futex', 'atomic_yield', 'socket', 'pipe']
# cpu_types = ['same', 'hyperthread', 'different']

def run_command(name):
    return int(subprocess.run(['./' + name], capture_output=True).stdout.decode('utf-8').strip())

def run_benchmark(name):
    return (float(median([run_command(name) for i in range(100)])) / num_iters) / 2

with open('benchmark.csv', mode='w') as bench_file:
    writer = csv.writer(bench_file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL) #, fieldnames=fieldnames)
    # writer.writeheader()
    writer.writerow(fieldnames)

    i = 1
    total = len(ipc_types) * 3
    for ipc in ipc_types:
        row = [ipc]
        print(f"{i}/{total} running {ipc}_same")
        row.append(run_benchmark(ipc + '_same'))
        i+=1

        print(f"{i}/{total} running {ipc}_hyperthread")
        row.append(run_benchmark(ipc + '_hyperthread'))
        i+=1

        print(f"{i}/{total} running {ipc}_different")
        row.append(run_benchmark(ipc + '_different'))
        i+=1

        writer.writerow(row)