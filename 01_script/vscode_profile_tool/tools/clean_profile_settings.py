"""clean_profile_settings.py

Remove settings in a VS Code profile's `settings.settings` that belong to extensions
which are NOT present in the profile `extensions` list.

Behavior summary:
- Keeps core VS Code settings (common prefixes like `editor`, `workbench`, ...).
- Keeps language-scoped settings (e.g. "[python]").
- Keeps settings whose prefix matches an installed extension's name (the substring
  after the last `.` in the extension id) OR the full extension id.
- Removes other dotted-prefixed settings (these are likely from extra extensions).

Usage:
  python tools/clean_profile_settings.py in.json [--out out.json] [--dry-run] [--report]

Features:
- --dry-run: don't write, just report
- --backup: write a backup of the input file
- --keep: add extra prefixes to always keep (can be supplied multiple times)

"""
from __future__ import annotations
import argparse
import json
import sys
from pathlib import Path
from typing import Dict, Iterable, List, Set, Tuple

# Common VS Code built-in prefixes we always keep
CORE_PREFIXES = {
    "editor",
    "workbench",
    "window",
    "files",
    "telemetry",
    "extensions",
    "explorer",
    "git",
    "terminal",
    "search",
    "breadcrumbs",
    "http",
    "update",
    "filesExclude",
    "files.associations",
    "files.autoGuessEncoding",
    "files.encoding",
    "breadcrumbs",
    "editorTokensColorCustomizations",
}


def load_profile(path: Path) -> Dict:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def find_extension_ids(profile: Dict) -> List[str]:
    """Return a list of extension ids found in the profile.

    Supports array entries that are either strings or objects with an
    `identifier.id` field (the format produced by VS Code profiles/exports).
    """
    ext_ids: List[str] = []
    raw_ext = profile.get("extensions") or []
    if isinstance(raw_ext, dict):
        # some exporters nest data; try common keys
        for k in ("recommendations", "extensions", "visible", "installed"):
            if k in raw_ext and isinstance(raw_ext[k], list):
                raw_ext = raw_ext[k]
                break

    for item in raw_ext:
        if isinstance(item, str):
            ext_ids.append(item)
        elif isinstance(item, dict):
            # Example shape: { "identifier": { "id": "publisher.name" }, ... }
            ident = item.get("identifier")
            if isinstance(ident, dict) and isinstance(ident.get("id"), str):
                ext_ids.append(ident["id"])
            else:
                # fallback: sometimes id is directly present
                if isinstance(item.get("id"), str):
                    ext_ids.append(item["id"])
    return ext_ids


def extension_prefixes(ext_ids: Iterable[str]) -> Tuple[Set[str], Set[str]]:
    """Return (full_ids, short_names) for matching setting-key prefixes.

    short_name is substring after the last '.' in the extension id.
    """
    full_ids = set()
    short_names = set()
    for eid in ext_ids:
        full_ids.add(eid)
        if "." in eid:
            short_names.add(eid.rsplit(".", 1)[1])
        else:
            short_names.add(eid)
    return full_ids, short_names


def is_language_scope(key: str) -> bool:
    # e.g. "[python]" or "[yaml]"
    return key.startswith("[") and key.endswith("]")


def key_prefixes_to_check(key: str) -> List[str]:
    # return candidate prefixes to match against known extension ids/names
    # - exact full id: startswith 'publisher.name.'
    # - short prefix: 'name.'
    # - top-level prefix (before first dot): 'prefix'
    prefixes: List[str] = []
    if "." in key:
        prefixes.append(key.split(".", 1)[0])
    else:
        prefixes.append(key)
    return prefixes


def clean_settings_obj(
    settings_obj: Dict[str, object],
    ext_full_ids: Set[str],
    ext_short_names: Set[str],
    keep_prefixes: Iterable[str],
) -> Tuple[Dict[str, object], List[str]]:
    keep_prefixes_set = set(keep_prefixes) | CORE_PREFIXES
    kept: Dict[str, object] = {}
    removed_keys: List[str] = []

    for k, v in settings_obj.items():
        # Always keep language-scoped and non-dotted top-level keys that are builtin
        if is_language_scope(k) or k in keep_prefixes_set:
            kept[k] = v
            continue

        # If key starts with any full extension id + '.' -> keep
        kept_flag = False
        for fid in ext_full_ids:
            if k.startswith(fid + "."):
                kept[k] = v
                kept_flag = True
                break
        if kept_flag:
            continue

        # If top-level prefix matches a known short extension name or a core prefix -> keep
        top = k.split(".", 1)[0]
        if top in ext_short_names or top in keep_prefixes_set:
            kept[k] = v
            continue

        # Otherwise consider it to be from an extra/removed extension
        removed_keys.append(k)

    return kept, removed_keys


def make_backup(path: Path) -> Path:
    bak = path.with_suffix(path.suffix + ".bak")
    path.replace(bak)
    return bak


def write_profile(path: Path, profile: Dict, *, indent: int = 2) -> None:
    with path.open("w", encoding="utf-8") as f:
        json.dump(profile, f, ensure_ascii=False, indent=indent)
        f.write("\n")


def main(argv: List[str]) -> int:
    p = argparse.ArgumentParser(
        prog="clean_profile_settings.py",
        description="Remove settings belonging to extensions not present in a VS Code profile",
    )
    p.add_argument("input", help="expanded profile JSON (e.g. output from expand_code_profile.py)")
    p.add_argument("--out", "-o", help="output file (default: overwrite input)")
    p.add_argument("--dry-run", action="store_true", help="don't write, only show report")
    p.add_argument("--backup", action="store_true", help="backup input before overwriting")
    p.add_argument(
        "--keep",
        action="append",
        default=[],
        help="additional top-level prefixes to always keep (repeatable)",
    )
    p.add_argument("--report", action="store_true", help="print removed keys and counts")
    args = p.parse_args(argv)

    inp = Path(args.input)
    if not inp.exists():
        print(f"ERROR: input file not found: {inp}", file=sys.stderr)
        return 2

    profile = load_profile(inp)

    # Navigate into nested 'settings' typical for code-profile files
    settings_container = profile.get("settings")
    if not isinstance(settings_container, dict) or "settings" not in settings_container:
        print("ERROR: expected profile to contain profile['settings']['settings']", file=sys.stderr)
        return 3

    settings_obj = settings_container.get("settings", {})
    if not isinstance(settings_obj, dict):
        print("ERROR: profile['settings']['settings'] is not an object", file=sys.stderr)
        return 4

    ext_ids = find_extension_ids(profile)
    ext_full_ids, ext_short_names = extension_prefixes(ext_ids)

    kept_settings, removed_keys = clean_settings_obj(
        settings_obj, ext_full_ids, ext_short_names, args.keep
    )

    if args.report or args.dry_run:
        print(f"Total settings: {len(settings_obj)}")
        print(f"Kept settings:  {len(kept_settings)}")
        print(f"Removed settings: {len(removed_keys)}")
        if removed_keys:
            print("\nRemoved keys (sample up to 200):")
            for k in removed_keys[:200]:
                print(" -", k)

    if args.dry_run:
        return 0

    out_path = Path(args.out) if args.out else inp
    if out_path == inp and args.backup:
        bak = inp.with_suffix(inp.suffix + ".bak")
        inp.replace(bak)
        out_path = inp
        # if backup is requested we already moved the original to .bak; write to original path
        # but keep a note
        print(f"Backup written to: {bak}")

    # update profile and write
    profile["settings"]["settings"] = kept_settings
    write_profile(out_path, profile)
    print(f"Wrote cleaned profile to: {out_path}  (removed {len(removed_keys)} settings)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
