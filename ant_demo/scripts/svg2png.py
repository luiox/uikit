#!/usr/bin/env uv
# uv add cairosvg
"""简单脚本：将某个目录下的所有 SVG 转换为 PNG
使用 uv 管理依赖，也可以手动安装 cairosvg 或使用 inkscape 命令行
"""
import os
import sys
from pathlib import Path

# when run directly with uv, uv will ensure cairosvg is available
# if running with plain python we still try an auto-install fallback

try:
    import cairosvg
except ImportError:
    cairosvg = None
    # fallback: attempt pip install if running under normal Python
    if not sys.argv[0].endswith("uv"):
        try:
            import subprocess
            print("cairosvg not found, installing via pip...")
            subprocess.check_call([sys.executable, "-m", "pip", "install", "cairosvg"])
            import cairosvg
        except Exception:
            cairosvg = None


def svg_to_png(svg_path: Path, png_path: Path, dpi=96):
    if cairosvg:
        cairosvg.svg2png(url=str(svg_path), write_to=str(png_path), dpi=dpi)
    else:
        # 尝试使用 inkscape
        os.system(f"inkscape '{svg_path}' --export-type=png --export-filename='{png_path}'")


def main():
    if len(sys.argv) < 3:
        print("Usage: svg2png.py <svg-dir> <out-dir>")
        return
    svg_dir = Path(sys.argv[1])
    out_dir = Path(sys.argv[2])
    out_dir.mkdir(parents=True, exist_ok=True)
    for svg in svg_dir.glob("*.svg"):
        png_file = out_dir / (svg.stem + ".png")
        print(f"converting {svg} -> {png_file}")
        svg_to_png(svg, png_file)

if __name__ == '__main__':
    main()
