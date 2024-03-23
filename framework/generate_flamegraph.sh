#!/bin/sh
# Usage
#       ./generate_flamegraph.sh binary_to_run fg_name
#
#       binary_to_run - program to trace
#       fg_name - name of flamegraph

sudo perf record -g $1
sudo perf script > out.perf
stackcollapse-perf.pl out.perf > out.folded

flamegraph.pl out.folded > $2.svg
