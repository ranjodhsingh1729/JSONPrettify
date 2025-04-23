#!/usr/bin/env python3
import argparse
import subprocess
import sys
import time
import statistics


def benchmark(command, runs):
    """
    Run the specified command multiple times and collect timing statistics.
    """
    timings = []
    for i in range(1, runs + 1):
        start = time.perf_counter()
        try:
            subprocess.run(command, check=True)
        except subprocess.CalledProcessError as e:
            sys.exit(e.returncode)
        elapsed = time.perf_counter() - start
        timings.append(elapsed)
        # print(f"Run #{i:2d}: {elapsed:.6f} sec")

    avg_time = statistics.mean(timings)
    min_time = min(timings)
    max_time = max(timings)
    stddev = statistics.stdev(timings) if runs > 1 else 0.0

    print("\n=== Benchmark Results ===")
    print(f"Total runs       : {runs}")
    print(f"Average time     : {avg_time:.6f} sec")
    print(f"Fastest time     : {min_time:.6f} sec")
    print(f"Slowest time     : {max_time:.6f} sec")
    print(f"Std. deviation   : {stddev:.6f} sec")


def main():
    parser = argparse.ArgumentParser(
        description="Benchmark an arbitrary binary or command by running it multiple times and reporting timing stats."
    )
    parser.add_argument(
        "-n", "--runs",
        type=int,
        default=10,
        help="Number of times to run the command (default: 10)"
    )
    parser.add_argument(
        "command", 
        nargs=argparse.REMAINDER,
        help="The binary or command (with args) to benchmark"
    )

    args = parser.parse_args()

    if not args.command:
        parser.error("Please specify the command to benchmark, e.g.: bench.py -n 20 -- ls -l")

    # On some shells the '--' is included in args.command; remove if present
    cmd = args.command
    if cmd[0] == "--":
        cmd = cmd[1:]

    benchmark(cmd, args.runs)


if __name__ == "__main__":
    main()
