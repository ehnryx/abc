from pathlib import Path
from typing import Optional

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


def main(args):
    contents = set(abc_list.get_contents(args))
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
            for line in all_lines:
                if line.strip() == "#pragma once":
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
                else:
                    ouf.write(line)

    with open(args.output, "w") as f:
        expand(args.input, f, before_pragma=False)


def register(subs):
    parser = subs.add_parser(
        "insert",
        aliases=["i"],
        description="Replace includes with code. Paths should be relative to abc_root",
        help="Replace includes with code. Paths should be relative to abc_root",
    )
    parser.add_argument("input", type=Path)
    parser.add_argument("--output", "-o", type=Path, default=Path("to_submit.cpp"))
    parser.set_defaults(func=main)
