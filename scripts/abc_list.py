from pathlib import Path
from typing import List


ignored_files = [
    "__pycache__",
    ".git",
    "beta",
    "tests",
]

valid_suffixes = [
    ".h",
    ".cpp",
]


def walk_dir(path: Path):
    if path.name in ignored_files:
        return
    if path.is_dir():
        for f in path.iterdir():
            yield from walk_dir(path=f)
    elif path.suffix in valid_suffixes:
        yield path.resolve()


def get_contents(args) -> List[Path]:
    return sorted(set(walk_dir(path=args.abc_root)))


def get_contents_relative(args) -> List[Path]:
    return [f.relative_to(args.abc_root) for f in get_contents(args)]


def main(args):
    print("\n".join(str(f) for f in get_contents_relative(args)))


def register(subs):
    parser = subs.add_parser(
        "list",
        aliases=["ls", "ll", "l"],
        description="List all header files",
        help="List all header files",
    )
    parser.set_defaults(func=main)
