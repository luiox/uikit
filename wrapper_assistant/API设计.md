# 启动项配置前后端接口设计

## 1. 配置数据结构（Rust端/前端通用）
```json
{
  "categories": [
    "Embedded",
    "IDE",
    ...
  ],
  "items": [
    {
      "category": "IDE",
      "name": "Visual Studio 2022",
      "target_path": "C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/devenv.exe",
      "icon_location": "C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/devenv.exe", // exe 路径或 base64
      "arguments": "",
      "count": 432,
      "icon_base64": "data:image/png;base64,..." // Rust端返回时可选
    },
    ...
  ]
}
```

## 2. 前后端接口设计

### 2.1 获取所有分类和启动项
- **前端调用**：`invoke('get_all_launch_items')`
- **Rust返回**：完整的分类和启动项列表，icon_location 可为 exe 路径或 base64

### 2.2 获取单个启动项的图标
- **前端调用**：`invoke('get_icon_base64', { path: 'C:/xxx/xxx.exe' })`
- **Rust返回**：`{ base64: 'data:image/png;base64,...' }`

### 2.3 新增/编辑/删除启动项
- **新增**：`invoke('add_launch_item', { item: { ... } })`
- **编辑**：`invoke('edit_launch_item', { old_name, item: { ... } })`
- **删除**：`invoke('remove_launch_item', { name, category })`
- **Rust端**：更新本地配置文件，返回最新数据

### 2.4 获取/保存配置文件
- **获取**：`invoke('get_config')` → 返回完整 config.json 内容
- **保存**：`invoke('save_config', { config })` → Rust端写入 config.json

### 2.5 启动程序

`invoke('launch_program',)`

## 3. 说明
- 图标字段 icon_location 前端传递 exe 路径，Rust端负责解析并返回 base64。
- 前端所有数据变更都通过 invoke 通知 Rust，Rust端负责持久化。
- 可根据实际需求扩展接口，如分类管理、排序等。
