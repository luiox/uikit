"""compact_code_profile.py

Reverse of expand_code_profile.py — take an expanded JSON profile and
stringify selected top-level keys so the result matches VS Code's
`.code-profile` export format (common: `settings`, `extensions`, `globalState`).

Usage examples:
  python tools/compact_code_profile.py tools/Test.code-profile.fixed.json
  python tools/compact_code_profile.py in.json -o out.code-profile --keys settings,extensions --double-encode settings
  python tools/compact_code_profile.py in.json --inplace --backup

Behavior (defaults):
- By default will stringify top-level `settings`, `extensions`, `globalState` if present.
- `--double-encode` can be used for keys (e.g. `settings`) that should be nested as
  `{ "settings": "<json-string>" }` and then stringified (matches some VS Code exports).
- Safe: if a target key is already a string, it is left unchanged unless `--force`.

The produced file is validated as JSON before writing.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path
from typing import Any, Iterable, Mapping


def stringify_value(val: Any, pretty_inner: bool = False) -> str:
    """Return a JSON string representation for val (dict/list/primitive).

    If val is already a string, assume it's already encoded and return it.
    """
    if isinstance(val, str):
        return val
    # Use compact separators by default to "compress" the inner JSON
    if pretty_inner:
        return json.dumps(val, ensure_ascii=False, indent=2)
    return json.dumps(val, ensure_ascii=False, separators=(",", ":"))


def compact_profile(obj: Mapping[str, Any],
                    keys: Iterable[str],
                    double_encode: Iterable[str] | None = None,
                    pretty_inner: bool = False,
                    force: bool = False) -> dict:
    out = dict(obj)
    de_keys = set(double_encode or [])
    for k in keys:
        if k not in obj:
            continue
        v = obj[k]
        # If already a string and not forcing, leave as-is
        if isinstance(v, str) and not force:
            out[k] = v
            continue
        # If double-encode requested for this key, produce {key: "<inner-json>"} and then stringify that
        if k in de_keys:
            # For settings specifically, allow either the whole value to be the inner dict
            # or value to be a dict that already contains a 'settings' key.
            if isinstance(v, dict) and 'settings' in v and isinstance(v['settings'], (dict, list)):
                inner = stringify_value(v['settings'], pretty_inner=pretty_inner)
                outer = {'settings': inner}
                out[k] = json.dumps(outer, ensure_ascii=False, separators=(",", ":"))
            else:
                # Wrap the entire value as a string inside an outer object named same as key
                inner = stringify_value(v, pretty_inner=pretty_inner)
                outer = {k: inner}
                out[k] = json.dumps(outer, ensure_ascii=False, separators=(",", ":"))
            continue
        # Normal single-level stringify (for arrays/objects)
        out[k] = stringify_value(v, pretty_inner=pretty_inner)
    return out


def load_json(path: Path) -> Any:
    return json.loads(path.read_text(encoding='utf-8'))


def write_text(path: Path, text: str) -> None:
    path.write_text(text, encoding='utf-8')


def parse_args(argv=None):
    p = argparse.ArgumentParser(description='Compact expanded profile to .code-profile format')
    p.add_argument('input', nargs='?', default='tools/Test.code-profile.fixed.json')
    p.add_argument('-o', '--output', help='output path (default: input with .repacked.code-profile)')
    p.add_argument('--keys', help='comma-separated top-level keys to stringify (default: settings,extensions,globalState)')
    p.add_argument('--double-encode', help='comma-separated keys to double-encode (e.g. settings)')
    p.add_argument('--pretty-inner', action='store_true', help='pretty-print inner JSON strings')
    p.add_argument('--inplace', action='store_true')
    p.add_argument('--backup', action='store_true', help='create .bak when --inplace')
    p.add_argument('--force', action='store_true', help='force-stringify even if value is already a string')
    p.add_argument('--dry-run', action='store_true', help='print result to stdout but do not write')
    p.add_argument('-v', '--verbose', action='store_true')
    return p.parse_args(argv)


def main(argv=None) -> int:
    args = parse_args(argv)
    inp = Path(args.input)
    if not inp.exists():
        print(f'ERROR: input not found: {inp}')
        return 2

    data = load_json(inp)
    default_keys = ['settings', 'extensions', 'globalState']
    keys = [k.strip() for k in (args.keys.split(',') if args.keys else default_keys) if k.strip()]
    de_keys = [k.strip() for k in (args.double_encode.split(',') if args.double_encode else []) if k.strip()]

    compacted = compact_profile(data, keys=keys, double_encode=de_keys, pretty_inner=args.pretty_inner, force=args.force)

    # Validate produced JSON
    try:
        _ = json.loads(json.dumps(compacted, ensure_ascii=False))
    except Exception as e:
        print('ERROR: result is not JSON-serializable:', e)
        return 3

    out_path = Path(args.output) if args.output else inp.with_name(inp.stem + '.repacked' + inp.suffix)

    if args.inplace:
        if args.backup:
            bak = inp.with_suffix(inp.suffix + '.bak')
            bak.write_bytes(inp.read_bytes())
            if args.verbose:
                print(f'backup written to: {bak}')
        if not args.dry_run:
            write_text(inp, json.dumps(compacted, ensure_ascii=False))
            if args.verbose:
                print(f'wrote (inplace) -> {inp}')
    else:
        if not args.dry_run:
            write_text(out_path, json.dumps(compacted, ensure_ascii=False))
            if args.verbose:
                print(f'wrote -> {out_path}')

    if args.dry_run or args.verbose:
        print(json.dumps(compacted, indent=2, ensure_ascii=False))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
