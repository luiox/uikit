import os
import subprocess
import sys

# --- 配置部分 ---

# 1. 指定 clang-format 可执行文件的路径
#    - 在 Windows 上，通常是 'clang-format.exe' 或完整路径，例如 'C:/path/to/clang-format.exe'
#    - 在 Linux/macOS 上，通常是 'clang-format' 或完整路径，例如 '/usr/local/bin/clang-format'
#    - 如果 'clang-format' 已在系统的 PATH 环境变量中，可以直接写 'clang-format'
CLANG_FORMAT_PATH = "tools/clang-format.exe"

# 2. 指定要格式化的源代码目录列表
#    脚本会从当前脚本所在的目录开始计算这些路径
#    这与原 Bash 脚本中 $(pwd)/src 的逻辑一致
SRC_DIRS = [
    "src",
    # 如果有更多目录，可以在这里添加，例如：
    # "include",
    # "lib/some_library",
]

# 3. 指定需要格式化的文件扩展名
FILE_EXTENSIONS = ('.h', '.c', '.cpp', '.hpp')

# --- 脚本主逻辑 ---

def main():
    """
    主函数，用于遍历目录并格式化代码文件。
    """
    # 获取脚本所在的根目录
    # os.path.dirname(__file__) 获取当前脚本文件所在的目录
    # os.path.abspath() 将其转换为绝对路径
    try:
        # __file__ 在直接执行脚本时是有效的
        root_dir = os.path.abspath(os.path.dirname(__file__))
    except NameError:
        # 如果在某些环境中 __file__ 未定义（例如交互式解释器），则使用当前工作目录
        root_dir = os.path.abspath(os.getcwd())
        print("警告: __file__ 未定义，将使用当前工作目录作为根目录。", file=sys.stderr)

    print(f"脚本根目录: {root_dir}")
    print(f"使用的 clang-format 路径: {CLANG_FORMAT_PATH}")

    # 检查 clang-format 是否存在
    # 首先检查是否是绝对路径或相对于脚本路径
    clang_format_full_path = os.path.join(root_dir, CLANG_FORMAT_PATH)
    if not os.path.isfile(clang_format_full_path):
        # 如果不是，则检查是否是系统PATH中的命令
        print(f"在 '{clang_format_full_path}' 未找到 clang-format，尝试在系统 PATH 中查找...")
        clang_format_full_path = CLANG_FORMAT_PATH
    
    # 验证 clang-format 是否可执行
    try:
        # 使用 --version 参数来检查工具是否可用且可执行
        subprocess.run([clang_format_full_path, "--version"], check=True, capture_output=True, text=True)
        print(f"成功找到可用的 clang-format: {clang_format_full_path}")
    except (subprocess.CalledProcessError, FileNotFoundError):
        print(f"错误: 无法找到或执行 clang-format 于 '{clang_format_full_path}'。", file=sys.stderr)
        print("请确保 CLANG_FORMAT_PATH 变量已正确设置。", file=sys.stderr)
        sys.exit(1) # 退出脚本，返回错误码

    # 遍历所有配置的源代码目录
    for src_dir_name in SRC_DIRS:
        # 构建源代码目录的完整绝对路径
        src_dir_full_path = os.path.join(root_dir, src_dir_name)

        # 检查目录是否存在
        if not os.path.isdir(src_dir_full_path):
            print(f"警告: 源目录 '{src_dir_full_path}' 不存在，已跳过。", file=sys.stderr)
            continue

        print(f"\n--- 开始扫描目录: {src_dir_full_path} ---")

        # os.walk 是 Python 中遍历目录树的标准方法
        for dirpath, _, filenames in os.walk(src_dir_full_path):
            for filename in filenames:
                # 检查文件扩展名是否在目标列表中（不区分大小写）
                if filename.lower().endswith(FILE_EXTENSIONS):
                    file_path = os.path.join(dirpath, filename)
                    
                    print(f"正在格式化: {file_path}")
                    
                    try:
                        # 使用 subprocess.run 执行命令
                        # [clang_format_full_path, '-i', file_path] 构造命令列表
                        # check=True: 如果命令返回非零退出码（表示错误），则抛出 CalledProcessError 异常
                        subprocess.run([clang_format_full_path, '-i', file_path], check=True)
                    except subprocess.CalledProcessError as e:
                        # 如果 clang-format 执行失败（例如，文件有语法错误）
                        print(f"错误: 格式化文件 '{file_path}' 失败。", file=sys.stderr)
                        print(f"  命令: {' '.join(e.cmd)}", file=sys.stderr)
                        print(f"  返回码: {e.returncode}", file=sys.stderr)
                    except FileNotFoundError:
                        # 这个理论上在前面已经检查过了，但作为双重保障
                        print(f"错误: 找不到命令 '{clang_format_full_path}'。", file=sys.stderr)
                        sys.exit(1)
    
    print("\n所有文件格式化完成。")

    try:
        input("\n按 Enter 键退出...")
    except (EOFError, KeyboardInterrupt):
        # 处理在输入时用户按下 Ctrl+C 或 Ctrl+D 的情况
        print("\n用户中断，退出。")


if __name__ == "__main__":
    main()
