#!/bin/env python3
import subprocess
import argparse
from statistics import fmean, pvariance,pstdev 

# def run_command(args):
#     return subprocess.run(args, capture_output=True).stdout.decode("utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="analyze.py",description="analyze benchmark times")

    parser.add_argument("filename")
    args = parser.parse_args()

    if args.filename is None:
        raise Exception

    with open(args.filename, encoding="utf-8") as f:
        contents = f.read()
        times = [float(x) for x in contents.split("\n") if len(x) > 0]

        print(f"mean = {fmean(times)}")
        print(f"stddev = {pstdev(times)}")
        print(f"variance = {pvariance(times)}")
