#pragma once

#include <cstddef>
#include <cstdint>

namespace iconlib {

enum class Icon : uint32_t {
    None = 0,
    Close,
    Minimize,
    Maximize,
    Restore,
    Help,
    Info,
    Warning,
    Error,
    Settings,
    Menu,
    File,
    Folder,
    FolderOpen,
    Image,
    Audio,
    Video,
    Archive,
    Pdf,
    Word,
    Excel,
    Ppt,
    Code,
    New,
    Open,
    Save,
    SaveAs,
    Delete,
    Edit,
    Copy,
    Paste,
    Cut,
    Undo,
    Redo,
    SelectAll,
    Find,
    Replace,
    Back,
    Forward,
    Up,
    Refresh,
    Home,
    ZoomIn,
    ZoomOut,
    Fullscreen,
    FullscreenExit,
    Grid,
    Search,
    Filter,
    Sort,
    SortAsc,
    SortDesc,
    Check,
    Circle,
    Square,
    Star,
    Heart,
    Flag,
    Tag,
    Clock,
    Calendar,
    Link,
    Unlink,
    Web,
    Download,
    Upload,
    Cloud,
    Share,
    External,
    Play,
    Pause,
    Stop,
    Prev,
    Next,
    Volume,
    VolumeMute,
    Mic,
    Computer,
    ControlPanel,
    Cmd,
    Registry,
    Shutdown,
    Restart,
    Lock,
    Unlock,
    Key,
    Printer,
    Battery,
    Wifi,
    App,
    Group,
    Tab,
    Pin,
    Unpin,
    Drag,
    Plus,
    Minus,
    History,
    Freq,
    SearchHistory,
    Clear,
    _Count
};

struct IconAsset {
    Icon icon;
    const char* id;
    const char* fileName;
    const char* svg;
    std::size_t size;
};

std::size_t GetIconCount();
const IconAsset* GetAllIcons();
const IconAsset* FindIcon(Icon icon);
const IconAsset* FindIcon(const char* id);

const char* GetDynamicPath(Icon icon);
const char* GetDynamicPath(const char* id);

const char* GetEmbeddedSvg(Icon icon);
const char* GetEmbeddedSvg(const char* id);

} // namespace iconlib
