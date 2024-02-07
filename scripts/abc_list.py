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


def main(args):
    contents = get_contents(args)
    print("\n".join(str(f.relative_to(args.abc_root)) for f in contents))


def register(subs):
    parser = subs.add_parser(
        "list",
        aliases=["ls", "ll", "l"],
        description="List all header files",
        help="List all header files",
    )
    parser.set_defaults(func=main)
