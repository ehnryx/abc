import argparse

from pathlib import Path

import abc_compile
import abc_find
import abc_insert
import abc_list
import abc_time


def parse_args():
    parser = argparse.ArgumentParser("abc: AutoBookCode")
    parser.add_argument("abc_root", type=Path, help=argparse.SUPPRESS)
    subparsers = parser.add_subparsers(
        title="valid commands",
        dest="command",
        required=True,
    )
    abc_compile.register(subparsers)
    abc_find.register(subparsers)
    abc_insert.register(subparsers)
    abc_list.register(subparsers)
    abc_time.register(subparsers)
    return parser.parse_args()


def main(args):
    args.func(args)


if __name__ == "__main__":
    main(parse_args())
