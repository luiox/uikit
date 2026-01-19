#!/usr/bin/env python3
"""
音频格式转换器
只需要修改下面的配置，然后直接运行即可
"""

import os
import sys
from pathlib import Path
import ffmpeg



# ================= 配置区域 (在这里修改) =================

# 输入路径 (可以是单个 m4a 文件，也可以是包含 m4a 文件的文件夹)
INPUT_PATH = r"demo.mp3"

# 输出格式 (可选: "mp3" 或 "wav")
OUT_FORMAT = "wav"

# 输出路径 (可选)
# 如果是单文件转换：填写输出的文件名 (包含后缀，如 output.mp3 或 output.wav)
# 如果是文件夹转换：填写输出文件夹路径
# 如果留空 (None)：自动在输入路径同目录生成，文件名不变，后缀改为指定格式
OUTPUT_PATH = None

# 音频比特率 (可选，默认 192k)
# 注意: wav 格式是无损的，此参数对 wav 无效，仅对 mp3 有效
BITRATE = '192k'

# 是否覆盖已存在的文件 (True=覆盖, False=跳过)
OVERWRITE = False


# =======================================================

def get_output_format_params():
    """根据输出格式返回对应的编码器参数"""
    fmt = OUT_FORMAT.lower()
    if fmt == 'mp3':
        # MP3 使用 libmp3lame 编码器
        return 'libmp3lame', '.mp3'
    elif fmt == 'wav':
        # WAV 使用 pcm_s16le 编码器 (无损)
        return 'pcm_s16le', '.wav'
    else:
        print(f"警告: 不支持的格式 '{OUT_FORMAT}'，默认使用 mp3")
        return 'libmp3lame', '.mp3'


def convert_file(input_file, output_file):
    """转换单个文件的核心逻辑"""

    # 获取编码器
    codec_name, ext = get_output_format_params()

    try:
        print(f"正在转换: {Path(input_file).name} -> {Path(output_file).name}")

        # 构建输出参数字典
        output_args = {
            'codec:a': codec_name,
            'map_metadata': '0'  # 保留封面和元数据
        }

        # 只有 MP3 需要设置比特率，WAV 是无损的不需要
        if OUT_FORMAT.lower() == 'mp3':
            output_args['b:a'] = BITRATE

        # 执行转换
        (
            ffmpeg
            .input(str(input_file))
            .output(str(output_file), **output_args)
            .overwrite_output()
            .run(quiet=True, overwrite_output=True)
        )
        return True
    except Exception as e:
        print(f"  [失败] {e}")
        return False


def main():
    # 1. 检查路径是否填写
    if INPUT_PATH == "请在这里粘贴您的文件路径或文件夹路径" or not INPUT_PATH:
        print("=" * 50)
        print("错误: 请先打开脚本修改顶部的 INPUT_PATH 变量！")
        print("=" * 50)
        return

    # 获取标准后缀名 (例如 .mp3 或 .wav)
    _, target_ext = get_output_format_params()

    input_p = Path(INPUT_PATH)

    # 2. 检查路径是否存在
    if not input_p.exists():
        print(f"错误: 找不到路径，请检查: {INPUT_PATH}")
        return

    # ================= 场景 A: 单文件转换 =================
    if input_p.is_file():
        print(f"检测到文件，开始单文件转换 (目标格式: {OUT_FORMAT})...")

        # 确定输出文件名
        if OUTPUT_PATH:
            # 如果用户指定了输出路径，直接使用
            out_p = Path(OUTPUT_PATH)
        else:
            # 否则修改原文件后缀
            out_p = input_p.with_suffix(target_ext)

        # 检查是否覆盖
        if out_p.exists() and not OVERWRITE:
            print(f"文件已存在，已跳过: {out_p}")
            return

        # 执行转换
        success = convert_file(input_p, out_p)
        if success:
            print(f"\n转换成功！保存位置: {out_p}")
        else:
            print("\n转换失败，请检查是否安装了 FFmpeg")

    # ================= 场景 B: 批量文件夹转换 =================
    elif input_p.is_dir():
        print(f"检测到文件夹，开始批量扫描 (目标格式: {OUT_FORMAT})...")

        # 设置输出目录
        if OUTPUT_PATH:
            out_dir = Path(OUTPUT_PATH)
            out_dir.mkdir(parents=True, exist_ok=True)
        else:
            out_dir = input_p

        # 查找所有 m4a (包含子文件夹)
        m4a_files = list(input_p.rglob('*.m4a'))

        if not m4a_files:
            print("未找到任何 .m4a 文件")
            return

        print(f"共找到 {len(m4a_files)} 个文件，准备转换...\n")

        success_count = 0
        for index, m4a_file in enumerate(m4a_files, 1):
            # 保持相对目录结构，并修改后缀
            relative_path = m4a_file.relative_to(input_p)
            mp3_file = out_dir / relative_path.with_suffix(target_ext)

            # 创建子文件夹
            mp3_file.parent.mkdir(parents=True, exist_ok=True)

            # 检查覆盖
            if mp3_file.exists() and not OVERWRITE:
                print(f"[{index}/{len(m4a_files)}] 跳过 (已存在): {relative_path.name}")
                continue

            # 转换
            if convert_file(m4a_file, mp3_file):
                success_count += 1
            else:
                pass

        print(f"\n批量转换完成！")
        print(f"总计: {len(m4a_files)} | 成功: {success_count} | 失败/跳过: {len(m4a_files) - success_count}")


if __name__ == '__main__':
    main()
