import os

def remove_substring(directory, substring):
    for root, dirs, files in os.walk(directory):
        # 处理文件夹名中的特定子串
        for dir_name in dirs[:]:
            if substring in dir_name:
                old_dir = os.path.join(root, dir_name)
                new_dir = os.path.join(root, dir_name.replace(substring, ''))
                os.rename(old_dir, new_dir)
                remove_substring(new_dir, substring)  # 递归处理子文件夹

        # 处理文件名中的特定子串
        for file in files:
            if substring in file:
                old_name = os.path.join(root, file)
                new_name = os.path.join(root, file.replace(substring, ''))
                os.rename(old_name, new_name)

# 示例用法
directory = './xxx'  # 替换为你要遍历的目录路径
substring = 'rubbish'  # 替换为你要去除的特定字串
remove_substring(directory, substring)