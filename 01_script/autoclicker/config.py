from dataclasses import dataclass, field
from typing import Optional, Tuple

@dataclass
class BasicConfig:
    click_button: str = "left"  # 'left' or 'right'
    interval: float = 0.5  # seconds
    click_point: Optional[Tuple[int, int]] = None
    # 目标模式: 'coord' = 使用输入坐标或 overlay/F9, 'cursor' = 使用当前鼠标位置（在开始时采样）
    target_mode: str = 'coord'
    # 加入基础模式安全相关设置
    restrict_to_window: bool = False
    target_window_title: str = ""
    stop_on_leave: bool = False

@dataclass
class AdvancedConfig:
    move_speed: float = 5.0
    move_range: int = 100
    click_interval: float = 0.5
    target_window_title: str = ""
    fail_safe: bool = True

@dataclass
class AppConfig:
    basic: BasicConfig = field(default_factory=BasicConfig)
    advanced: AdvancedConfig = field(default_factory=AdvancedConfig)
