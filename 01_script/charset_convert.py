import os
import chardet
import codecs

# 使用此脚本需要安装chardet库
# pip install chardet
# 替换为需要修改编码的文件的目录路径
directory_path = R'.'
# 目标的编码
target_encoding = 'utf-8'
# 文件后缀过滤
file_suffixes = ['.cpp', '.hpp', '.c', '.h']

def detect_encoding(file_path):
    with open(file_path, 'rb') as f:
        result = chardet.detect(f.read())
        return result['encoding']

def convert_file_encoding(file_path, source_encoding, target_encoding):
    with codecs.open(file_path, 'r', source_encoding) as source_file:
        content = source_file.read()
    with codecs.open(file_path, 'w', target_encoding) as target_file:
        target_file.write(content)

def convert_directory(directory):
    for root, dirs, files in os.walk(directory):
        for filename in files:
            if any(filename.endswith(suffix) for suffix in file_suffixes):
                file_path = os.path.join(root, filename)
                source_encoding = detect_encoding(file_path)
                print(f"Detected encoding '{source_encoding}' for file '{file_path}'. Converting to '{target_encoding}'...")
                convert_file_encoding(file_path, source_encoding, target_encoding)

convert_directory(directory_path)
