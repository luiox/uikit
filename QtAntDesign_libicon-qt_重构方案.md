# QtAntDesign 基于 libicon-qt 的重构方案

## 1. 目标

本方案的目标不是直接把所有 `:/Imgs/...` 一次性替换掉，而是先把 QtAntDesign 的图标访问方式收口到统一入口：

- 核心图标资产由 `libicon-core` 提供
- Qt 侧图标适配由 `libicon-qt` 提供
- 控件层统一通过 `icon::qt::IconManager` 获取 `QIcon` / `QPixmap`

这样做的价值是：

1. 去掉控件层对 `qrc` 字符串路径的强耦合
2. 让多个 Qt 项目共享同一套 icon 资产
3. 为后续拆分 QtAntDesign 的 demo 和组件库打基础
4. 为主题切换、图标缓存、染色策略预留统一扩展点

---

## 2. 新的资源分层

建议形成三层：

### 2.1 `libicon-core`

职责：

- 维护标准 icon catalog
- 提供 `icon::Icon`
- 提供 `icon::FindIcon()` / `icon::GetEmbeddedSvg()` / `icon::GetDynamicPath()`

这层不依赖 Qt。

### 2.2 `libicon-qt`

职责：

- 提供 `icon::qt::IconManager`
- `icon::Icon -> QIcon`
- `icon::Icon -> QPixmap`
- SVG 染色
- 内存缓存

这层是 Qt 适配层，负责把图标核心库转换成 Qt 可直接消费的 UI 资源。

### 2.3 `QtAntDesign`

职责：

- 只表达控件逻辑、交互逻辑、主题逻辑
- 不再直接依赖 `:/Imgs/...`
- 不再在控件内部随处 new `QSvgRenderer("...path...")`

---

## 3. 推荐的最终图标调用方式

原来的写法：

```cpp
QIcon(":/Imgs/search.svg")
QPixmap(":/Imgs/bee.png")
QSvgRenderer(QStringLiteral(":/Imgs/downArrow.svg"), this)
```

重构后建议：

```cpp
using icon::qt::IconManager;

auto& icons = IconManager::instance();
QIcon searchIcon = icons.icon(icon::Icon::Search, QSize(16, 16));
QPixmap closePm = icons.pixmap(icon::Icon::Close, QSize(14, 14));
```

如果某个控件依赖 `QSvgRenderer`，建议也不要直接吃路径字符串，而是先从 `IconManager` 获取 `QPixmap`，或者后续增加：

- `QByteArray svg(icon::Icon id, QColor tint)`

由控件统一走内存数据渲染。

---

## 4. QtAntDesign 中应该先改哪些地方

## Phase 1：先接管核心系统级图标

优先改这些模块：

1. `DesignSystem`
2. `AntMessage`
3. `AntInput`
4. `AntComboBox`
5. `AntTabWidget`
6. `AntTreeView`
7. `CustomToolButton`
8. `AntButton`

原因：

- 这些是组件库级别公共能力
- 复用频率高
- 替换收益最大
- 修改后能立刻减少大量 `:/Imgs/...` 字符串散布

### 4.1 `DesignSystem` 改造建议

当前 `DesignSystem` 既管理主题，又返回一堆图标路径字符串。这一层应改成返回：

- `QIcon`
- 或者返回语义化图标枚举

推荐方案：

#### 方案 A：`DesignSystem` 直接返回 `QIcon`

```cpp
QIcon DesignSystem::btnMinIcon() const;
QIcon DesignSystem::btnCloseIcon() const;
```

内部统一：

```cpp
return icon::qt::IconManager::instance().icon(icon::Icon::Minimize, QSize(16, 16), color);
```

优点：

- 调用侧最简单
- 替换成本低

缺点：

- `DesignSystem` 会更偏 UI 资源层

#### 方案 B：`DesignSystem` 返回语义角色

```cpp
enum class UiIconRole {
    WindowMin,
    WindowMax,
    WindowClose,
    NavHome,
    NavSettings,
    MsgInfo,
    MsgWarning,
    MsgError
};
```

再由一个专门映射层把 `UiIconRole -> icon::Icon + tint + size`。

优点：

- 语义最清晰
- 后续主题/品牌/皮肤切换更容易

缺点：

- 初期改造量更大

**建议**：QtAntDesign 第一阶段先用方案 A，第二阶段再过渡到方案 B。

---

## 5. 具体替换策略

### 5.1 第一批直接替换的资源类型

适合直接迁到 `libicon-core` + `libicon-qt` 的：

- 搜索图标
- 菜单图标
- 关闭/最大化/最小化/还原图标
- 消息提示图标（info / warning / error / success）
- 通用箭头图标
- 设置 / 首页 / 返回 等通用图标

### 5.2 第一阶段不要迁的内容

不要一开始就迁这些：

- JPG 轮播图
- 品牌 Logo
- demo 插画
- 示例头像 PNG
- 视频资源
- 字体资源

这些不应该进入 `libicon-core`。

更合理的做法是：

- 组件级图标走 `libicon-core`
- demo 级素材继续留在 QtAntDesign demo 工程

---

## 6. 对控件 API 的建议

当前一些控件 API 仍以路径字符串为中心，例如：

- `setSvgIcon(const QString& iconPath)`
- `setSvgIcons(const QString& normalPath, const QString& activePath)`

建议逐步演化为：

```cpp
void setIconId(icon::Icon iconId);
void setIconIds(icon::Icon normal, icon::Icon active);
void setIconRole(UiIconRole role);
```

如果需要兼容旧项目，可以保留旧接口，但内部转发到新接口：

```cpp
void setSvgIcon(const QString& iconPath); // deprecated
void setIconId(icon::Icon iconId);        // preferred
```

这样可以渐进迁移，不必一次性全部改完。

---

## 7. `icon::qt::IconManager` 在 QtAntDesign 中的落点

推荐增加一个薄封装，例如：

- `QtAntDesign/IconProvider.h`
- `QtAntDesign/IconProvider.cpp`

职责：

- 把 `QtAntDesign` 内部“语义角色”映射到 `icon::Icon`
- 统一定义默认尺寸
- 统一定义亮/暗主题 tint 策略
- 封装 `IconManager::instance()` 的直接调用

示意：

```cpp
class IconProvider {
public:
    static QIcon navHome(bool active, bool dark);
    static QIcon messageInfo(bool dark);
    static QPixmap closePixmap(QSize size, bool dark);
};
```

这一层的好处：

- QtAntDesign 不直接绑定 `libicon-core` 的具体 icon catalog 名称
- 以后换 catalog、补 icon、改映射，不影响控件层

---

## 8. 推荐实施顺序

### 第一步：接入 `libicon-qt`

在 QtAntDesign 的构建系统中增加：

- `add_deps("libicon_qt")`
- Qt 模块保留 `Core/Gui/Widgets/Svg`

### 第二步：新增 `IconProvider`

先不要全局替换，先做中间层。

### 第三步：优先替换 `DesignSystem`

把这些能力换掉：

- 窗口控制按钮
- 导航栏图标
- 消息提示图标
- 通用箭头图标

### 第四步：替换核心控件

- `AntInput`
- `AntComboBox`
- `AntMessage`
- `AntButton`
- `CustomToolButton`
- `AntTabWidget`
- `AntTreeView`

### 第五步：清理旧 `qrc`

当迁移完成后，再逐步从 `QtAntDesign.qrc` 删除已经不用的通用图标。

---

## 9. 工作量评估

### 9.1 小步可交付版本

内容：

- 接入 `libicon_qt`
- 新增 `IconProvider`
- 改造 `DesignSystem`
- 替换窗口按钮和导航图标

预计：**2 ~ 4 人日**

### 9.2 核心控件迁移版本

内容：

- 替换主要控件的图标输入接口
- 减少大部分 `:/Imgs/...` 图标依赖
- 保留 demo 资源不动

预计：**5 ~ 9 人日**

### 9.3 完整库化版本

内容：

- demo / library 彻底拆分
- 旧路径接口降级或移除
- qrc 资源清理
- 文档更新

预计：**10 ~ 18 人日**

---

## 10. 最终建议

推荐路线不是“直接全替换”，而是：

1. 用 `libicon-core` 统一 icon 资产
2. 用 `libicon-qt` 统一 Qt 图标适配
3. 在 QtAntDesign 里增加 `IconProvider` 语义层
4. 先改 `DesignSystem`，再改控件，最后清理 `qrc`

一句话概括：

> `libicon-core` 解决“图标资产复用”，`libicon-qt` 解决“Qt 图标消费方式统一”，`IconProvider` 解决“QtAntDesign 语义与底层资源解耦”。
