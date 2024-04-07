import subprocess

from pathlib import Path
from typing import Optional, Set

import abc_list


# returns absolute path
def get_include(line) -> Optional[Path]:
    if (
        (line.startswith("#include") or line.startswith("%:include"))
        and not "// abc skip" in line
        and (include := line.split()[1]).startswith('"')
    ):
        return Path(include[1 : include.find('"', 1)])
    return None


def get_contents(args) -> Set[Path]:
    all_contents = set(abc_list.get_contents(args))
    if args.defines is None:
        return all_contents

    compiler_cmd = f"g++ -MM -I {args.abc_root} {args.input}" + "".join(
        " -D " + d for d in args.defines
    )
    print(f"getting included headers with {compiler_cmd}")
    compiler_run = subprocess.run(compiler_cmd.split(), capture_output=True, text=True)
    if compiler_run.returncode != 0:
        print(compiler_run.stderr.strip())
        exit(compiler_run.returncode)
    preprocessed_includes = {
        Path(x).resolve()
        for x in compiler_run.stdout.split()[2:]  # [object, source, headers...]
        if x != "\\"  # sometimes included as line breaks
    }
    print("got included headers: " + " ".join(str(x) for x in preprocessed_includes))
    return all_contents & preprocessed_includes


def main(args):
    assert args.input.is_file(), f"input file {args.input} does not exist"

    if args.no_defines:
        assert len(args.defines) == 0, "--defines cannot be used with --no-defines"
        args.defines = None

    contents = get_contents(args)
    inserted = set()

    def expand(path: Path, ouf, *, before_pragma: bool):
        if path in inserted:
            ouf.write("// already included\n")
            return
        inserted.add(path)

        print(f"copying {path}")
        with open(path, "r") as inf:
            all_lines = list(inf)
            if before_pragma and not any(line.strip() == "#pragma once" for line in all_lines):
                before_pragma = False  # no `#pragma once`, include the entire file
            if not before_pragma:
                ouf.write(f'#line 1 "{path}"\n')
            for lnum, line in enumerate(all_lines):
                if line.strip() == "#pragma once":
                    ouf.write(f'#line {lnum + 2} "{path}"\n')
                    before_pragma = False
                    continue
                if before_pragma:
                    continue
                if (include := get_include(line)) is not None and (
                    absolute_include := (args.abc_root / include).resolve()
                ) in contents:
                    ouf.write(f"// START {line}")
                    expand(absolute_include, ouf, before_pragma=True)
                    ouf.write(f"// END {line}")
                    ouf.write(f'#line {lnum + 2} "{path}"\n')
                else:
                    ouf.write(line)

    with open(args.output, "w") as ouf:
        if args.defines is not None:
            for d in args.defines:
                ouf.write(f"#define {d}\n")
        expand(args.input, ouf, before_pragma=False)
    print(f"written to {args.output}")


def register(subs):
    parser = subs.add_parser(
        "insert",
        aliases=["i"],
        description="Replace includes with code. Paths should be relative to abc_root",
        help="Replace includes with code. Paths should be relative to abc_root",
    )
    parser.add_argument("input", type=Path)
    parser.add_argument("--output", "-o", type=Path, default=Path("to_submit.cpp"))
    parser.add_argument("--no-defines", action="store_true")
    parser.add_argument(
        "--defines",
        "-d",
        nargs="*",
        default=[],  # use defines by default
        help="respect the preprocessor and use defines (-D)",
    )
    parser.set_defaults(func=main)
