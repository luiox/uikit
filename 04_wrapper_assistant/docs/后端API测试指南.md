# 后端 API 测试指南

## 测试准备

### 1. 启动开发服务器
```bash
cd src-tauri
cargo run
```

### 2. 或者使用 Tauri 开发模式
```bash
pnpm tauri:dev
```

## API 命令测试

你可以通过前端调用这些命令，或者创建测试页面来验证：

### 1. 获取配置
```typescript
const config = await invoke('get_all_launch_items')
console.log('当前配置:', config)
```

### 2. 添加测试程序
```typescript
const testItem = {
  name: "记事本",
  target_path: "notepad.exe",
  icon_location: "",
  arguments: "",
  count: 0
}

await invoke('add_launch_item', {
  category: "系统工具",
  item: testItem
})
```

### 3. 启动程序
```typescript
const request = {
  target_path: "notepad.exe",
  arguments: undefined,
  run_as_admin: false
}

await invoke('launch_program', { req: request })
```

### 4. 管理员启动测试
```typescript
const adminRequest = {
  target_path: "cmd.exe",
  arguments: undefined,
  run_as_admin: true
}

await invoke('launch_program', { req: adminRequest })
```

### 5. 获取程序图标
```typescript
const iconBase64 = await invoke('get_icon_base64', { 
  path: "C:/Windows/System32/notepad.exe" 
})
console.log('图标数据:', iconBase64)
```

### 6. 扫描系统程序
```typescript
const programs = await invoke('scan_system_programs')
console.log('扫描到的程序:', programs)
```

### 7. 更新使用次数
```typescript
await invoke('update_usage_count', {
  name: "记事本",
  category: "系统工具"
})
```

### 8. 删除程序
```typescript
await invoke('remove_launch_item', {
  name: "记事本",
  category: "系统工具"
})
```

## 预期功能验证

### ✅ 应该正常工作的功能：
1. **程序启动** - 普通模式和管理员模式
2. **图标提取** - 从 exe 文件提取图标
3. **配置管理** - 保存和读取 JSON 配置
4. **路径处理** - `%%rp%%` 和 `%%rr%%` 变量替换

### 🔍 重点测试项：
1. **Windows 系统程序扫描** - 检查是否能正确扫描到程序
2. **管理员权限启动** - 测试 UAC 提权是否正常
3. **图标提取性能** - 大量程序时的图标提取速度
4. **配置文件持久化** - 重启后配置是否保持

## 调试技巧

### 1. 查看 Tauri 日志
开发模式下，Rust 的 `println!` 输出会显示在终端

### 2. 添加调试日志
```rust
println!("调试: 启动程序 {:?}", req.target_path);
```

### 3. 检查配置文件
配置保存在 `config.json`，可以直接查看内容

### 4. 错误处理测试
尝试错误的路径或参数，确保错误信息正确返回

## 常见问题排查

### 问题1：程序启动失败
- 检查路径是否正确
- 检查文件是否存在
- 检查权限问题

### 问题2：图标提取失败
- 确保是 Windows 系统
- 检查 exe 文件是否有图标
- 验证 Windows API 调用

### 问题3：配置保存失败
- 检查文件写入权限
- 验证 JSON 序列化
- 确认配置目录存在

### 问题4：扫描程序太慢
- 限制扫描范围
- 异步处理
- 添加进度提示
