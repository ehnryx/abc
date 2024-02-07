from difflib import SequenceMatcher

import abc_list


def main(args):
    L = len(args.pattern)

    matches = []
    for f in map(str, abc_list.get_contents_relative(args)):
        ratio = SequenceMatcher(args.pattern, f).ratio()
        for i in range(len(f) - L + 1):
            ratio = max(ratio, SequenceMatcher(a=args.pattern, b=f[i : i + L]).ratio())
        if ratio > args.tolerance:
            matches.append((f, ratio))

    for f, ratio in sorted(matches, reverse=True, key=lambda x: x[1]):
        if args.verbose:
            print(f"{ratio:0.2f}: {f}")
        else:
            print(f)


def register(subs):
    parser = subs.add_parser(
        "find",
        description=(
            "fuzzy search for pattern,"
            " similarity ratio is taken from python's difflib.SequenceMatcher."
            " default tolerance is 0.8"
        ),
        help=(
            "fuzzy search for pattern,"
            " similarity ratio is taken from python's difflib.SequenceMatcher."
            " default tolerance is 0.8"
        ),
    )
    parser.add_argument("pattern", type=str)
    parser.add_argument("--tolerance", "-t", type=float, default=0.8)
    parser.add_argument("--verbose", "-v", action="store_true")
    parser.set_defaults(func=main)
