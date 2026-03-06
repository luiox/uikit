# libicon-qt

`libicon-qt` 是 `libicon-core` 的 Qt 适配层。

## 提供能力

- `icon::qt::IconManager`
- `icon::Icon -> QIcon`
- `icon::Icon -> QPixmap`
- SVG 染色
- 内存缓存

## 目标

给 Qt 项目提供统一的图标访问入口，避免控件层直接写：

- `QIcon(":/Imgs/xxx.svg")`
- `QPixmap(":/Imgs/xxx.svg")`
- `QSvgRenderer(":/Imgs/xxx.svg")`

## 接口示例

```cpp
#include "IconManager.h"

using icon::qt::IconManager;

QIcon searchIcon = IconManager::instance().icon(icon::Icon::Search, QSize(16, 16));
QPixmap closePm = IconManager::instance().pixmap(icon::Icon::Close, QSize(14, 14));
```

## xmake

在主工程中引入子项目后：

- `add_deps("libicon_qt")`

当前 `libicon_qt` 默认依赖：

- `libicon_core_embed`
- `QtCore`
- `QtGui`
- `QtSvg`
