import subprocess

from pathlib import Path


def main(args):
    if args.test is None:
        args.test = args.input.with_suffix(".in")
        print(f"Guessing input file to be '{args.test}'")
    assert args.input.exists(), f"input binary '{args.input}' does not exist"
    assert args.test.exists(), f"test file '{args.test}' does not exist"

    cmd = f"{args.abc_root}/scripts/abc_time.sh {args.input} {args.test}".split()
    print(f"Running `{cmd}` {args.runs} times")
    all_runs = []
    for it in range(args.runs):
        res = subprocess.run(cmd, capture_output=True, text=True)
        runtime = float(res.stdout.strip())
        print(f"{runtime} on run {it + 1} of {args.runs}")
        all_runs.append(runtime)
    all_runs.sort()

    print("\nResults ...........................")
    print(f"Average: {sum(all_runs) / args.runs}")
    for i in [2, 4, 5, 6, 8]:
        value = all_runs[i * args.runs // 10]
        print(f"{i}0%: {value}")


def register(subs):
    parser = subs.add_parser(
        "time",
        aliases=["t"],
        description="Times the code. gives median over some number of runs",
        help="Times the code. gives median over some number of runs",
    )
    parser.add_argument("input", type=Path, help="binary to time")
    parser.add_argument("--runs", "-n", type=int, help="number of runs", required=True)
    parser.add_argument(
        "--test", "-t", type=Path, help="file that contains test input", default=None
    )
    parser.set_defaults(func=main)
