"""expand_code_profile.py

Read a VS Code `.code-profile` file and recursively decode any JSON-encoded
strings (handles double- or multi-encoded fields such as the common
`settings` -> `settings` string). Produces a prettified JSON output file
(or can overwrite the original with backup).

Usage examples:
  python tools/expand_code_profile.py tools/Test.code-profile
  python tools/expand_code_profile.py tools/Test.code-profile -o tools/Test.code-profile.fixed.json
  python tools/expand_code_profile.py tools/Test.code-profile --inplace --backup

Features:
- Repeatedly attempts json.loads on string values up to `--max-depth` times.
- Recursively decodes dicts and lists.
- Safe defaults: writes to a `.fixed.json` file unless `--inplace` is used.
- Verbose / dry-run modes for inspection.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path
from typing import Any, Iterable


def try_json_loads_once(s: str):
    """Attempt a single json.loads on s — return (success, result_or_error)."""
    try:
        return True, json.loads(s)
    except Exception as e:
        return False, e


def deep_decode(value: Any, max_depth: int = 10) -> Any:
    """Recursively decode JSON-encoded strings inside value.

    - If `value` is a string and contains JSON, repeatedly json.loads until
      it no longer decodes to a string/dict/list or max_depth is reached.
    - If `value` is a dict or list, recurse into elements.
    - Otherwise, return value unchanged.
    """
    # Protect against pathological depth
    if max_depth <= 0:
        return value

    # If it's a string, try repeatedly decoding
    if isinstance(value, str):
        current = value
        for _ in range(max_depth):
            success, parsed = try_json_loads_once(current)
            if not success:
                break
            # If parsed is the same string (rare), stop
            if parsed == current:
                current = parsed
                break
            current = parsed
            # If parsed is a string again, loop to try decode further
            if isinstance(current, str):
                continue
            # If parsed is dict/list/other, recursively decode its contents
            return deep_decode(current, max_depth=max_depth - 1)
        # Could not parse (or reached depth); return original string
        return current

    # If dict -> decode values
    if isinstance(value, dict):
        out = {}
        for k, v in value.items():
            out[k] = deep_decode(v, max_depth=max_depth - 1)
        return out

    # If list/tuple -> decode items
    if isinstance(value, list):
        return [deep_decode(i, max_depth=max_depth - 1) for i in value]
    if isinstance(value, tuple):
        return tuple(deep_decode(i, max_depth=max_depth - 1) for i in value)

    # Other types are returned as-is
    return value


def expand_profile(obj: dict, target_keys: Iterable[str] | None = None, max_depth: int = 10) -> dict:
    """Return a copy of `obj` with JSON-encoded string fields expanded.

    - If `target_keys` is provided, only attempts decode on those top-level keys.
    - Otherwise, attempts to decode all string values found recursively.
    """
    if target_keys is None:
        return deep_decode(obj, max_depth=max_depth)

    out = dict(obj)
    for k in target_keys:
        if k in obj:
            out[k] = deep_decode(obj[k], max_depth=max_depth)
    return out


def load_profile(path: Path) -> dict:
    raw = path.read_text(encoding="utf-8")
    try:
        return json.loads(raw)
    except json.JSONDecodeError as e:
        raise SystemExit(f"Failed to parse {path!s} as JSON: {e}") from e


def write_json(obj: Any, path: Path, pretty: bool = True) -> None:
    path.write_text(json.dumps(obj, indent=2 if pretty else None, ensure_ascii=False) + "\n", encoding="utf-8")


def parse_args(argv: Iterable[str] | None = None):
    p = argparse.ArgumentParser(description="Expand a VS Code .code-profile to readable JSON")
    p.add_argument("input", nargs="?", default="tools/Test.code-profile", help="path to .code-profile file")
    p.add_argument("-o", "--output", help="output path (default: input + .fixed.json)")
    p.add_argument("--inplace", action="store_true", help="replace the input file (creates backup when --backup)")
    p.add_argument("--backup", action="store_true", help="when used with --inplace create a .bak copy first")
    p.add_argument("--keys", help="comma-separated top-level keys to decode only (e.g. settings,extensions,globalState)")
    p.add_argument("--max-depth", type=int, default=12, help="max recursive decode attempts (default: 12)")
    p.add_argument("--dry-run", action="store_true", help="print result to stdout but do not write file")
    p.add_argument("--pretty/--no-pretty", dest="pretty", default=True)
    p.add_argument("-v", "--verbose", action="store_true")
    return p.parse_args(argv)


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(argv)
    inp = Path(args.input)
    if not inp.exists():
        print(f"ERROR: input file not found: {inp}")
        return 2

    profile = load_profile(inp)
    keys = None
    if args.keys:
        keys = [k.strip() for k in args.keys.split(",") if k.strip()]

    expanded = expand_profile(profile, target_keys=keys, max_depth=args.max_depth)

    if args.dry_run or args.verbose:
        print(json.dumps(expanded, indent=2, ensure_ascii=False))

    out_path = Path(args.output) if args.output else inp.with_suffix(inp.suffix + ".fixed.json")

    if args.inplace:
        if args.backup:
            bak = inp.with_suffix(inp.suffix + ".bak")
            bak.write_bytes(inp.read_bytes())
            if args.verbose:
                print(f"backup written to: {bak}")
        if not args.dry_run:
            write_json(expanded, inp, pretty=args.pretty)
            if args.verbose:
                print(f"wrote expanded profile (in-place) -> {inp}")
    else:
        if not args.dry_run:
            write_json(expanded, out_path, pretty=args.pretty)
            if args.verbose:
                print(f"wrote expanded profile -> {out_path}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
