import argparse
import pty

from pathlib import Path


def get_compile_command(args):
    code = args.input if args.input.suffix else args.input.with_suffix(".cpp")
    binary = code.stem if args.output is None else args.output
    cmd = (
        f"g++ -std=c++20 {code} -o {binary} -I {args.abc_root}"
        " -O2 -g -fmax-errors=1 -Wall -Wextra -Wshadow -Wconversion"
    ).split()
    if args.opt_info:
        cmd += "-fopt-info-vec-optimized -fopt-info-loop-optimized".split()
    if args.debug:
        cmd += "-O0 -ggdb -fsanitize=address,undefined -fno-omit-frame-pointer -D DEBUG".split()
    return cmd + args.extra_args


def main(args):
    cmd = get_compile_command(args)
    print(f"Running `{cmd}`")
    pty.spawn(cmd)


def register(subs):
    parser = subs.add_parser(
        "compile",
        aliases=["c"],
        description="Replace includes with code. Paths should be relative to abc_root",
        help="Replace includes with code. Paths should be relative to abc_root",
    )
    parser.add_argument("input", type=Path, help="file to compile. default extension is .cpp")
    parser.add_argument(
        "--output", "-o", default=None, help="binary output. default is input minus extension"
    )
    parser.add_argument(
        "--opt-info", "-i", action="store_true", help="add optimization info flags"
    )
    parser.add_argument("--debug", "-g", action="store_true", help="add debug flags")
    parser.add_argument(
        "--extra-args",
        "-e",
        nargs=argparse.REMAINDER,
        default=[],
        help="extra args for the compiler",
    )
    parser.set_defaults(func=main)