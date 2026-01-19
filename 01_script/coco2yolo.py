import json
import os
from PIL import Image
import numpy as np

# --- 配置 ---
coco_json_path = 'annotations_coco.json'  # 你的COCO JSON文件路径
images_dir = 'images/'                   # 你的图像所在目录
output_dir = 'labels_yolo/'              # 输出YOLO格式标签的目录
os.makedirs(output_dir, exist_ok=True)    # 确保输出目录存在

# --- 加载COCO数据 ---
with open(coco_json_path, 'r') as f:
    coco_data = json.load(f)

# 创建类别ID到类别名称的映射
category_id_to_name = {cat['id']: cat['name'] for cat in coco_data['categories']}
# 创建类别名称到类别ID的映射 (YOLO需要类别ID从0开始)
category_name_to_id = {cat['name']: idx for idx, cat in enumerate(coco_data['categories'])}
print(f"类别映射: {category_name_to_id}")

# 创建图像ID到图像信息的映射
image_id_to_info = {img['id']: img for img in coco_data['images']}

# --- 处理每个标注 ---
for annotation in coco_data['annotations']:
    image_id = annotation['image_id']
    category_id = annotation['category_id']
    category_name = category_id_to_name[category_id]
    segmentation = annotation['segmentation']  # COCO的多边形数据，通常是 [[x1,y1,x2,y2,...], ...]

    # 获取图像信息以获取图像尺寸
    image_info = image_id_to_info[image_id]
    image_width = image_info['width']
    image_height = image_info['height']
    image_file_name = image_info['file_name']
    image_basename = os.path.splitext(image_file_name)[0]  # 获取不带扩展名的文件名

    # 获取第一个多边形（假设每个标注只有一个多边形，或者只处理第一个）
    # COCO格式中segmentation可能是一个列表，包含多个多边形，这里简单处理第一个
    polygon = segmentation[0]
    polygon = np.array(polygon, dtype=np.float32).reshape(-1, 2)  # 转换为Nx2的数组

    # 计算最小外接矩形 (MBR)
    # 使用OpenCV的minAreaRect，它返回旋转后的最小矩形，但我们只需要轴对齐的矩形
    # 所以先找到多边形的边界点，再计算轴对齐的边界框
    min_x = np.min(polygon[:, 0])
    max_x = np.max(polygon[:, 0])
    min_y = np.min(polygon[:, 1])
    max_y = np.max(polygon[:, 1])

    # 计算矩形框的坐标 (COCO格式: [x_min, y_min, width, height])
    x_min = min_x
    y_min = min_y
    box_width = max_x - min_x
    box_height = max_y - min_y

    # 转换为YOLO格式: [class_id, center_x, center_y, width, height] (归一化到0-1)
    # 注意：这里假设类别ID从0开始，与category_name_to_id一致
    class_id = category_name_to_id[category_name]
    center_x = (x_min + box_width / 2.0) / image_width
    center_y = (y_min + box_height / 2.0) / image_height
    norm_width = box_width / image_width
    norm_height = box_height / image_height

    # 写入YOLO格式的TXT文件
    yolo_line = f"{class_id} {center_x:.6f} {center_y:.6f} {norm_width:.6f} {norm_height:.6f}\n"

    # 每个图像对应一个标签文件
    label_file_path = os.path.join(output_dir, f"{image_basename}.txt")

    # 将该标注添加到对应的标签文件中（如果文件已存在，则追加）
    with open(label_file_path, 'a') as label_file:
        label_file.write(yolo_line)

print("转换完成！YOLO格式的标签文件已保存到:", output_dir)
