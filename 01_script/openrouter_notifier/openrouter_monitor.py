#!/usr/bin/env python3
"""
OpenRouter 新模型监控脚本
功能：每隔1小时检查OpenRouter模型列表，发现变化时弹窗通知
"""

import json
import time
import requests
from datetime import datetime
from pathlib import Path
import platform
import subprocess
# 在 Windows 下启动时隐藏控制台窗口（使用 ctypes）；如果需要查看控制台可删除此段或设置环境变量
if platform.system() == "Windows":
    try:
        import ctypes
        SW_HIDE = 0
        hwnd = ctypes.windll.kernel32.GetConsoleWindow()
        if hwnd:
            ctypes.windll.user32.ShowWindow(hwnd, SW_HIDE)
    except Exception as e:
        # 隐藏失败不影响主程序运行
        print(f"[调试] 隐藏控制台失败: {e}")
# ============== 配置 ==============
CHECK_INTERVAL = 3600  # 检查间隔（秒），默认1小时
DATA_FILE = Path(__file__).parent / "openrouter_models.json"
OPENROUTER_API = "https://openrouter.ai/api/v1/models"

# ============== 弹窗通知 ==============
def send_notification(title: str, message: str):
    """根据操作系统发送桌面通知"""
    system = platform.system()
    
    if system == "Darwin":  # macOS
        # 使用 osascript 发送通知
        script = f'display notification "{message}" with title "{title}"'
        subprocess.run(["osascript", "-e", script], check=False)
        
    elif system == "Windows":
        # Windows 使用 plyer 或 win10toast
        try:
            from plyer import notification
            notification.notify(
                title=title,
                message=message,
                app_name="OpenRouter Monitor",
                timeout=10
            )
        except ImportError:
            # 备选方案：使用 PowerShell
            ps_script = f'''
            [Windows.UI.Notifications.ToastNotificationManager, Windows.UI.Notifications, ContentType = WindowsRuntime] | Out-Null
            [Windows.Data.Xml.Dom.XmlDocument, Windows.Data.Xml.Dom.XmlDocument, ContentType = WindowsRuntime] | Out-Null
            $template = @"
            <toast>
                <visual>
                    <binding template="ToastText02">
                        <text id="1">{title}</text>
                        <text id="2">{message}</text>
                    </binding>
                </visual>
            </toast>
"@
            $xml = New-Object Windows.Data.Xml.Dom.XmlDocument
            $xml.LoadXml($template)
            $toast = [Windows.UI.Notifications.ToastNotification]::new($xml)
            [Windows.UI.Notifications.ToastNotificationManager]::CreateToastNotifier("OpenRouter Monitor").Show($toast)
            '''
            subprocess.run(["powershell", "-Command", ps_script], check=False)
            
    elif system == "Linux":
        # Linux 使用 notify-send
        subprocess.run(["notify-send", title, message], check=False)
    
    print(f"[通知] {title}: {message}")


def show_windows_messagebox(title: str, message: str, max_chars: int = 8000):
    """在 Windows 上使用原生 MessageBox 显示模型详情；会在消息过长时截断以防卡死或超出显示限制。"""
    try:
        import ctypes
        MB_OK = 0x00000000
        # 截断过长消息以免影响显示
        if len(message) > max_chars:
            message = message[: max_chars - 20] + "\n... (truncated)"
        ctypes.windll.user32.MessageBoxW(0, str(message), str(title), MB_OK)
    except Exception as e:
        print(f"[警告] 无法显示 MessageBox: {e}")


# ============== API 请求 ==============
def fetch_models() -> dict:
    """从OpenRouter API获取模型列表"""
    try:
        response = requests.get(OPENROUTER_API, timeout=30)
        response.raise_for_status()
        data = response.json()
        return data
    except requests.RequestException as e:
        print(f"[错误] 获取模型列表失败: {e}")
        return None


def parse_models(data: dict) -> dict:
    """解析API响应，提取模型信息"""
    if not data or "data" not in data:
        return {}
    
    models = {}
    for model in data["data"]:
        model_id = model.get("id", "")
        models[model_id] = {
            "name": model.get("name", model_id),
            "description": model.get("description", ""),
            "context_length": model.get("context_length", 0),
            "pricing": model.get("pricing", {}),
            "top_provider": model.get("top_provider", {}),
        }
    return models


# ============== 数据存储 ==============
def load_stored_models() -> dict:
    """加载本地存储的模型数据"""
    if DATA_FILE.exists():
        try:
            with open(DATA_FILE, "r", encoding="utf-8") as f:
                data = json.load(f)
                return data.get("models", {})
        except (json.JSONDecodeError, IOError) as e:
            print(f"[警告] 读取存储文件失败: {e}")
    return {}


def save_models(models: dict):
    """保存模型数据到本地"""
    data = {
        "last_updated": datetime.now().isoformat(),
        "total_models": len(models),
        "models": models
    }
    with open(DATA_FILE, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
    print(f"[保存] 已保存 {len(models)} 个模型到 {DATA_FILE}")


# ============== 变化检测 ==============
def compare_models(old_models: dict, new_models: dict) -> dict:
    """比较新旧模型列表，返回变化"""
    old_ids = set(old_models.keys())
    new_ids = set(new_models.keys())
    
    added = new_ids - old_ids
    removed = old_ids - new_ids
    
    # 检测模型信息变化（可选）
    changed = set()
    for model_id in old_ids & new_ids:
        if old_models[model_id] != new_models[model_id]:
            changed.add(model_id)
    
    return {
        "added": sorted(list(added)),
        "removed": sorted(list(removed)),
        "changed": sorted(list(changed))
    }


# ============== 主循环 ==============
def check_once():
    """执行一次检查"""
    print(f"\n{'='*50}")
    print(f"[检查] {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    
    # 获取最新模型列表
    data = fetch_models()
    if not data:
        return
    
    new_models = parse_models(data)
    print(f"[信息] 当前共有 {len(new_models)} 个模型")
    
    # 加载之前的数据
    old_models = load_stored_models()
    
    if old_models:
        # 比较变化
        changes = compare_models(old_models, new_models)
        
        added = changes["added"]
        removed = changes["removed"]
        changed = changes["changed"]
        
        if added or removed or changed:
            # 有变化，发送通知
            message_parts = []
            
            if added:
                message_parts.append(f"新增 {len(added)} 个模型")
                print(f"[新增] {added}")
                for model_id in added[:5]:  # 最多显示5个
                    print(f"  - {model_id}: {new_models[model_id]['name']}")
                if len(added) > 5:
                    print(f"  ... 还有 {len(added) - 5} 个")
            
            if removed:
                message_parts.append(f"移除 {len(removed)} 个模型")
                print(f"[移除] {removed}")
            
            if changed:
                message_parts.append(f"更新 {len(changed)} 个模型")
                print(f"[更新] {changed}")
            
            # 发送弹窗通知（摘要）
            title = "🔔 OpenRouter 模型变化"
            message = " | ".join(message_parts)
            send_notification(title, message)

            # Windows 下显示详细 MessageBox（包含新增/移除/更新的具体 ID 与名称）
            if platform.system() == "Windows" and (added or removed or changed):
                details_lines = []
                if added:
                    details_lines.append("新增：")
                    for mid in added:
                        name = new_models.get(mid, {}).get("name", "")
                        details_lines.append(f"  + {mid} — {name}")
                if removed:
                    details_lines.append("移除：")
                    for mid in removed:
                        name = old_models.get(mid, {}).get("name", "")
                        details_lines.append(f"  - {mid} — {name}")
                if changed:
                    details_lines.append("更新：")
                    for mid in changed:
                        name = new_models.get(mid, {}).get("name", "")
                        details_lines.append(f"  * {mid} — {name}")

                details = "\n".join(details_lines) if details_lines else "(无详情)"
                show_windows_messagebox("OpenRouter 模型详情", details)

            # 继续保留现有的短通知（针对少量新增模型）
            if added and len(added) <= 3:
                for model_id in added:
                    model_name = new_models[model_id]['name']
                    send_notification(
                        f"🆕 新模型: {model_id}",
                        f"{model_name}"
                    )
        else:
            print("[结果] 无变化")
    else:
        print("[初始化] 首次运行，保存模型列表")
    
    # 保存最新数据
    save_models(new_models)


def main():
    """主函数"""
    print("="*50)
    print("🚀 OpenRouter 新模型监控脚本")
    print(f"📅 启动时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"⏱️  检查间隔: {CHECK_INTERVAL // 60} 分钟")
    print(f"📁 数据文件: {DATA_FILE}")
    print("="*50)
    
    # 发送启动通知
    send_notification("🚀 OpenRouter 监控已启动", f"每隔 {CHECK_INTERVAL // 60} 分钟检查一次")
    
    while True:
        try:
            check_once()
        except Exception as e:
            print(f"[异常] {e}")
            send_notification("⚠️ OpenRouter 监控异常", str(e))
        
        # 等待下次检查
        next_check = datetime.now().timestamp() + CHECK_INTERVAL
        next_time = datetime.fromtimestamp(next_check).strftime('%H:%M:%S')
        print(f"[等待] 下次检查时间: {next_time}")
        
        time.sleep(CHECK_INTERVAL)


if __name__ == "__main__":
    main()
