"""vscode-profile-tool — cross-platform profile exporter/importer

Features:
- list profiles (JSON files excluding Settings/KeyBoards)
- export <name> -> writes <name>.json (extensions only by default)
- import <name> -> installs extensions (by default does NOT import Settings/KeyBoards)

Usage examples:
  python main.py list
  python main.py export current --with-settings
  python main.py import python

Author: generated for user
"""

from __future__ import annotations
import argparse
import json
import os
import shutil
import subprocess
import sys
from datetime import datetime
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional

# ----- utils -----

def vscode_user_dir() -> Path:
    """Return the VS Code User settings directory for the current platform."""
    if sys.platform.startswith("win"):
        return Path(os.environ.get("APPDATA", "")) / "Code" / "User"
    if sys.platform == "darwin":
        return Path.home() / "Library" / "Application Support" / "Code" / "User"
    # linux
    return Path.home() / ".config" / "Code" / "User"


def run_code_command(args: List[str]) -> subprocess.CompletedProcess:
    cmd = ["code"] + args
    try:
        return subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=False)
    except FileNotFoundError:
        raise RuntimeError("'code' CLI not found in PATH; extension operations will be unavailable")


def backup(path: Path) -> Optional[Path]:
    if not path.exists():
        return None
    ts = datetime.now().strftime("%Y%m%d-%H%M%S")
    bak = path.with_name(path.name + ".bak." + ts)
    shutil.copy2(path, bak)
    return bak


def read_json(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8"))


def write_json(path: Path, obj: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(obj, ensure_ascii=False, indent=2), encoding="utf-8")


# ----- profile IO -----

def find_profiles(search_paths: Iterable[Path]) -> List[Path]:
    out: List[Path] = []
    for p in search_paths:
        if not p.exists():
            continue
        for f in p.glob("*.json"):
            if f.name.lower() in {"settings.json", "keyboards.json", "settings.json".lower(), "keyboards.json".lower()}:
                continue
            if f.name in ("Settings.json", "KeyBoards.json"):
                continue
            out.append(f)
    # uniq and sort by name
    seen = set()
    res = []
    for f in sorted(out, key=lambda p: p.name.lower()):
        if f.name in seen:
            continue
        seen.add(f.name)
        res.append(f)
    return res


def load_profile(path: Path) -> Dict[str, Any]:
    # support legacy plain-list files (one extension per line) as well as JSON
    txt = path.read_text(encoding="utf-8").strip()
    if not txt:
        return {}
    try:
        return json.loads(txt)
    except Exception:
        # treat as line-based extensions list
        exts = [line.split("@")[0].strip() for line in txt.splitlines() if line.strip() and not line.strip().startswith("#")]
        return {"name": path.stem, "extensions": exts}


# ----- export / import -----

def export_profile(name: str, out_dir: Path, with_settings: bool = False, with_keybindings: bool = False) -> Path:
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / f"{name}.json"

    profile: Dict[str, Any] = {"name": name}

    # extensions
    try:
        cp = run_code_command(["--list-extensions"])
        if cp.returncode == 0:
            exts = [line.strip() for line in cp.stdout.splitlines() if line.strip()]
        else:
            exts = []
    except RuntimeError:
        exts = []
    profile["extensions"] = exts

    user_dir = vscode_user_dir()
    if with_settings:
        sfile = user_dir / "settings.json"
        if sfile.exists():
            profile["settings"] = json.loads(sfile.read_text(encoding="utf-8"))
    if with_keybindings:
        kfile = user_dir / "keybindings.json"
        if kfile.exists():
            profile["keybindings"] = json.loads(kfile.read_text(encoding="utf-8"))

    write_json(out_path, profile)
    print(f"Exported profile -> {out_path}")
    return out_path


def install_extensions(exts: Iterable[str], dry_run: bool = False) -> None:
    for e in exts:
        e = e.split("@")[0].strip()
        if not e:
            continue
        print(f"Installing: {e}")
        if dry_run:
            continue
        cp = run_code_command(["--install-extension", e])
        if cp.returncode != 0:
            print(cp.stderr.strip())


def merge_settings(existing: Dict[str, Any], incoming: Dict[str, Any]) -> Dict[str, Any]:
    out = dict(existing)
    out.update(incoming)
    return out


def merge_keybindings(existing: List[Dict[str, Any]], incoming: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
    seen = set()
    out: List[Dict[str, Any]] = []
    for kb in existing + incoming:
        key = (kb.get("key"), kb.get("command"), kb.get("when"))
        if key in seen:
            continue
        seen.add(key)
        out.append(kb)
    return out


def import_profile(path: Path, with_settings: bool = False, with_keybindings: bool = False, dry_run: bool = False) -> None:
    profile = load_profile(path)
    exts = profile.get("extensions", [])
    if exts:
        install_extensions(exts, dry_run=dry_run)
    else:
        print("No extensions to install in profile.")

    user_dir = vscode_user_dir()
    # settings
    if with_settings and profile.get("settings"):
        sfile = user_dir / "settings.json"
        bak = backup(sfile)
        print(f"Backed up settings: {bak}") if bak else None
        existing = json.loads(sfile.read_text(encoding="utf-8")) if sfile.exists() else {}
        merged = merge_settings(existing, profile["settings"])
        if not dry_run:
            write_json(sfile, merged)
            print(f"Settings merged into {sfile}")

    # keybindings
    if with_keybindings and profile.get("keybindings"):
        kfile = user_dir / "keybindings.json"
        bak = backup(kfile)
        print(f"Backed up keybindings: {bak}") if bak else None
        existing = json.loads(kfile.read_text(encoding="utf-8")) if kfile.exists() else []
        merged = merge_keybindings(existing, profile["keybindings"])
        if not dry_run:
            write_json(kfile, merged)
            print(f"Keybindings merged into {kfile}")

    print(f"Import complete (profile: {path}).")


# ----- CLI -----

def _read_ext_list(path: Path) -> List[str]:
    if not path.exists():
        return []
    return [line.split('@')[0].strip() for line in path.read_text(encoding='utf-8').splitlines() if line.strip() and not line.strip().startswith('#')]


def repo_install(profile: str, repo_root: Path, with_settings: bool = False, with_keybindings: bool = False, dry_run: bool = False) -> None:
    """Install extensions from repo/extensions/common.txt and repo/extensions/<profile>.txt and optionally merge settings/keybindings fragments."""
    ext_dir = repo_root / 'extensions'
    settings_dir = repo_root / 'settings'
    keybindings_dir = repo_root / 'keybindings'

    common = _read_ext_list(ext_dir / 'common.txt')
    profile_list = _read_ext_list(ext_dir / f"{profile}.txt")
    to_install = list(dict.fromkeys(common + profile_list))
    if to_install:
        print(f"Installing {len(to_install)} extensions from repo ({repo_root})...")
        install_extensions(to_install, dry_run=dry_run)
    else:
        print("No extensions found in repo for this profile.")

    # merge settings fragments (common first, profile overrides)
    fragments: List[Path] = []
    if (settings_dir / 'Settings.json').exists():
        fragments.append(settings_dir / 'Settings.json')
    if (settings_dir / f"{profile}.json").exists():
        fragments.append(settings_dir / f"{profile}.json")
    if with_settings and fragments:
        existing = {}
        sfile = vscode_user_dir() / 'settings.json'
        if sfile.exists():
            existing = json.loads(sfile.read_text(encoding='utf-8'))
        merged = existing
        for f in fragments:
            merged = merge_settings(merged, json.loads(f.read_text(encoding='utf-8')))
        if not dry_run:
            bak = backup(sfile)
            write_json(sfile, merged)
            print(f"Merged settings written to {sfile} (backup: {bak})")

    # merge keybindings
    kb_fragments: List[Path] = []
    if (keybindings_dir / 'KeyBoards.json').exists():
        kb_fragments.append(keybindings_dir / 'KeyBoards.json')
    if (keybindings_dir / f"{profile}.json").exists():
        kb_fragments.append(keybindings_dir / f"{profile}.json")
    if with_keybindings and kb_fragments:
        kfile = vscode_user_dir() / 'keybindings.json'
        existing_kb = json.loads(kfile.read_text(encoding='utf-8')) if kfile.exists() else []
        incoming = []
        for f in kb_fragments:
            incoming += json.loads(f.read_text(encoding='utf-8'))
        merged_kb = merge_keybindings(existing_kb, incoming)
        if not dry_run:
            bak = backup(kfile)
            write_json(kfile, merged_kb)
            print(f"Merged keybindings written to {kfile} (backup: {bak})")

    print("repo-install complete.")


def repo_dedupe(repo_root: Path) -> None:
    """Compute intersection across extensions/*.txt (excluding common.txt), write common.txt and remove common items from each profile (with backups)."""
    ext_dir = repo_root / 'extensions'
    files = [p for p in ext_dir.glob('*.txt') if p.name != 'common.txt']
    if len(files) < 2:
        print('Need at least 2 profile files to dedupe.')
        return
    sets = {}
    for f in files:
        lst = _read_ext_list(f)
        sets[f] = set(lst)
    # intersection
    inter = set.intersection(*sets.values()) if sets else set()
    (ext_dir / 'common.txt').write_text('\n'.join(sorted(inter)) + ('\n' if inter else ''), encoding='utf-8')
    print(f'Wrote common.txt ({len(inter)} items)')
    # remove common from each and backup
    for f, s in sets.items():
        unique = sorted(s - inter)
        bak = f.with_name(f.name + '.bak.' + datetime.now().strftime('%Y%m%d-%H%M%S'))
        shutil.copy2(f, bak)
        f.write_text('\n'.join(unique) + ('\n' if unique else ''), encoding='utf-8')
        print(f'Updated {f.name}: {len(unique)} remaining (backup: {bak.name})')
    print('repo-dedupe complete.')


def repo_export(name: str, repo_root: Path, include_settings: bool = False, include_keybindings: bool = False) -> None:
    """Export current VS Code state to repo-format files: extensions/<name>.txt and optional settings/<name>.json, keybindings/<name>.json."""
    profile = export_profile(name, Path('.'), with_settings=include_settings, with_keybindings=include_keybindings)
    ext_dir = repo_root / 'extensions'
    settings_dir = repo_root / 'settings'
    keybindings_dir = repo_root / 'keybindings'
    ext_dir.mkdir(parents=True, exist_ok=True)
    settings_dir.mkdir(parents=True, exist_ok=True)
    keybindings_dir.mkdir(parents=True, exist_ok=True)
    # write extensions list (no versions)
    exts = profile.get('extensions', [])
    (ext_dir / f"{name}.txt").write_text('\n'.join([e.split('@')[0] for e in exts]) + ('\n' if exts else ''), encoding='utf-8')
    print(f'Wrote {ext_dir / (name + ".txt")} ({len(exts)} extensions)')
    if include_settings and profile.get('settings'):
        write_json(settings_dir / f"{name}.json", profile['settings'])
        print(f'Wrote settings/{name}.json')
    if include_keybindings and profile.get('keybindings'):
        write_json(keybindings_dir / f"{name}.json", profile['keybindings'])
        print(f'Wrote keybindings/{name}.json')
    print('repo-export complete.')


# ----- CLI (extended: repo-*) -----
    
def main(argv: Optional[List[str]] = None) -> int:
    p = argparse.ArgumentParser(prog="vscode-profile-tool", description="Manage flattened VS Code profiles and repository-style fragments")
    sub = p.add_subparsers(dest="cmd")

    sub_list = sub.add_parser("list", help="List available profile JSON files")
    sub_list.add_argument("paths", nargs="*", default=["."], help="Directories to search")

    sub_export = sub.add_parser("export", help="Export current VS Code state to a flattened profile JSON")
    sub_export.add_argument("name", help="Profile name / output filename (without .json)")
    sub_export.add_argument("--out", "-o", default=".", help="Output directory")
    sub_export.add_argument("--with-settings", action="store_true", help="Include settings.json in export")
    sub_export.add_argument("--with-keybindings", action="store_true", help="Include keybindings.json in export")

    sub_import = sub.add_parser("import", help="Import a profile JSON into VS Code (defaults: only extensions)")
    sub_import.add_argument("profile", help="Profile name or path (without .json searches repository/cwd)")
    sub_import.add_argument("--with-settings", action="store_true", help="Also import settings.json from profile")
    sub_import.add_argument("--with-keybindings", action="store_true", help="Also import keybindings.json from profile")
    sub_import.add_argument("--dry-run", action="store_true", help="Show actions without making changes")

    sub_repo_install = sub.add_parser("repo-install", help="Install a profile from repository fragments (extensions + optional settings/keybindings)")
    sub_repo_install.add_argument("profile", help="Profile name (matches extensions/<profile>.txt)")
    sub_repo_install.add_argument("--repo", default='.', help="Repository root containing extensions/, settings/, keybindings/")
    sub_repo_install.add_argument("--with-settings", action="store_true")
    sub_repo_install.add_argument("--with-keybindings", action="store_true")
    sub_repo_install.add_argument("--dry-run", action="store_true")

    sub_repo_export = sub.add_parser("repo-export", help="Export current VS Code state into repository fragments (extensions/<name>.txt, settings/<name>.json)")
    sub_repo_export.add_argument("name", help="Profile name to write into repo")
    sub_repo_export.add_argument("--repo", default='.', help="Repository root to write into")
    sub_repo_export.add_argument("--with-settings", action="store_true")
    sub_repo_export.add_argument("--with-keybindings", action="store_true")

    sub_repo_dedupe = sub.add_parser("repo-dedupe", help="Extract common extensions from multiple extensions/*.txt into extensions/common.txt and remove duplicates from profiles")
    sub_repo_dedupe.add_argument("--repo", default='.', help="Repository root containing extensions/")

sub_repo_assemble = sub.add_parser("repo-assemble", help="Assemble a .code-profile from repo fragments or explicit files")
sub_repo_assemble.add_argument("profile", help="Profile name (reads extensions/<profile>.txt, settings/<profile>.json, keybindings/<profile>.json)")
sub_repo_assemble.add_argument("--out", default=None, help="Output .code-profile path (default: <profile>.code-profile)")
sub_repo_assemble.add_argument("--with-extensions", action="store_true")
sub_repo_assemble.add_argument("--with-settings", action="store_true")
sub_repo_assemble.add_argument("--with-keybindings", action="store_true")
sub_repo_assemble.add_argument("--dry-run", action="store_true")

    args = p.parse_args(argv)
    if args.cmd == "list":
        dirs = [Path(pth) for pth in args.paths]
        profiles = find_profiles(dirs)
        for f in profiles:
            print(f.name)
        return 0

    if args.cmd == "export":
        out = export_profile(args.name, Path(args.out), with_settings=args.with_settings, with_keybindings=args.with_keybindings)
        print(out)
        return 0

    if args.cmd == "import":
        cand = Path(args.profile)
        if not cand.exists():
            cand = Path(args.profile + ".json")
        if not cand.exists():
            cwd = Path.cwd()
            cand = cwd / (args.profile + ".json")
        if not cand.exists():
            raise SystemExit(f"Profile not found: {args.profile}")
        import_profile(cand, with_settings=args.with_settings, with_keybindings=args.with_keybindings, dry_run=args.dry_run)
        return 0

    if args.cmd == 'repo-install':
        repo_install(args.profile, Path(args.repo), with_settings=args.with_settings, with_keybindings=args.with_keybindings, dry_run=args.dry_run)
        return 0

    if args.cmd == 'repo-export':
        repo_export(args.name, Path(args.repo), include_settings=args.with_settings, include_keybindings=args.with_keybindings)
        return 0

    if args.cmd == 'repo-dedupe':
        repo_dedupe(Path(args.repo))
        return 0

    if args.cmd == 'repo-assemble':
        out_path = Path(args.out) if args.out else Path(f"{args.profile}.code-profile")
        # lazy import of the assembler
        from tools.assemble_code_profile import main as _assemble_main
        assemble_args = ["--from-repo", args.profile]
        if args.with_extensions:
            assemble_args.append("--with-extensions")
        if args.with_settings:
            assemble_args.append("--with-settings")
        if args.with_keybindings:
            assemble_args.append("--with-keybindings")
        if args.dry_run:
            assemble_args.append("--dry-run")
        assemble_args += ["-o", str(out_path)]
        _assemble_main(assemble_args)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
