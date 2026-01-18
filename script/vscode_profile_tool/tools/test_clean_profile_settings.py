# simple unit tests for clean_profile_settings.py
import json
from pathlib import Path

from clean_profile_settings import (
    clean_settings_obj,
    extension_prefixes,
    find_extension_ids,
)


def make_sample_profile():
    return {
        "settings": {
            "settings": {
                "editor.fontSize": 14,
                "doxdocgen.generic.authorEmail": "a@b.c",
                "someOtherExt.option": True,
                "python.linting.enabled": True,
                "[python]": {"editor.tabSize": 4},
            }
        },
        "extensions": [
            {"identifier": {"id": "cschlosser.doxdocgen"}},
            {"identifier": {"id": "ms-python.python"}},
        ],
    }


def test_clean_keeps_known_and_extension_settings():
    p = make_sample_profile()
    ext_ids = find_extension_ids(p)
    full, short = extension_prefixes(ext_ids)
    kept, removed = clean_settings_obj(p["settings"]["settings"], full, short, [])
    assert "editor.fontSize" in kept
    assert "doxdocgen.generic.authorEmail" in kept
    assert "python.linting.enabled" in kept
    assert "[python]" in kept
    assert "someOtherExt.option" not in kept


if __name__ == "__main__":
    test_clean_keeps_known_and_extension_settings()
    print("ok")
