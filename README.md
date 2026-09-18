# uikit

duilib 的**视觉预制层**：主题（深/浅）、统一风格常用控件、无边框窗口预制件。
解决的是界面库（[DuiLib_DuiEditor fork](https://github.com/luiox/DuiLib_DuiEditor)）
不解决的三件事，整体作为 [luiox-repo](https://github.com/luiox/luiox-repo) 的
`uikit` 包对外，供 mlaunch 等项目消费。

## 三层结构

| 层 | 内容 | 落点 | 依赖 |
|---|---|---|---|
| L1 令牌数据 | 色板/字体/间距/圆角 + 图标语义映射 | `design/*.json` | 无 |
| L2 主题引擎 | 令牌解析校验、状态色派生、明暗方向、内嵌默认主题 | `core/`（纯 C++17） | nlohmann_json |
| L3 应用与控件 | 主题应用器、统一控件、无边框窗口预制件 | `duilib/` | duilib 包 |

设计纪律：**L1 令牌里禁止出现 duilib 属性语法**；`core/` 不 include 任何
`duilib/` 头（框架无关性由构建边界钉死，CI 在 linux 上以 `--core_only=y`
构建并跑测作为门禁）。

## 使用

```lua
-- xmake.lua
add_repositories("luiox-repo https://github.com/luiox/luiox-repo.git")
add_requires("uikit 0.1.0")

target("app")
    add_packages("uikit")   -- 纯引擎消费加 {configs = {core_only = true}}
```

```cpp
#include "uikit/duilib/frameless.h"
using namespace uikit::duilib;

class MyWindow : public FramelessWindow {
    LPCTSTR GetWindowClassName() const override { return _T("MyWindow"); }
    DuiLib::CControlUI* BuildRootUi() override {
        auto* root = new PanelUI();               // 主题化根容器
        auto* bar = new TitleBarUI();             // 无边框标题栏
        auto* check = new CheckBoxUI();           // 现代复选框
        check->SetText(_T("hello"));
        bar->Add(check);
        root->Add(bar);
        return root;
    }
    void Notify(DuiLib::TNotifyUI& msg) override {
        if (msg.sType == _T("click") && msg.pSender->GetName() == _T("toggle_theme")) {
            SetActiveTheme(ActiveTheme().appearance == color::Appearance::Light
                               ? DarkTheme() : LightTheme());
            ApplyThemeToTree(paint_manager().GetRoot());   // 整树实时换肤
        }
    }
};

// main: CoInitialize 后
MyWindow window; window.Run(_T("app"), 560, 440);
```

深浅两套默认主题**编译期内嵌**（`design/*.json` → `tools/gen_embed.sh` →
`core/include/uikit/embedded_themes.h`，改令牌后重新生成本文件，CI 校验同步），
运行期不依赖包安装路径；外部 json 可作换肤覆盖入口：
`ThemeTokens::parse(json, tokens, err)` + `resolve(tokens)` + `SetActiveTheme(...)`。

## 控件清单（当前）

- `ButtonUI`（状态色自绘；`StylePrimary/StyleSecondary` 跟随主题）、`IconButtonUI`、`MakeTextButton`
- `CheckBoxUI` / `RadioButtonUI`（自绘：accent 勾选盒/外环圆点，DPI 缩放，圆角）
- `LabelUI`（正文/弱化双角色，实时取主题色）、`PanelUI`（主题化面板容器）
- `SearchBoxUI`、`TitleBarUI`、`GroupListUI` / `ItemListUI` / `GroupRowUI`、`ApplyFlatScrollbar`
- `FramelessWindow`（无边框窗口基类：剥 caption + 吞 NC 区 + sizebox 缩放 + caption 拖拽/控件放行）

fork 缺口说明：此 fork 的 CButtonUI 状态色只能走图片、CCheckBoxUI 勾选态依赖
图片资源，故按钮族/勾选族在 `PaintStatusImage` 按主题色自绘；其余能力
（无边框消息语义、DPI）仍归 fork，uikit 不重包。

## 准入法则：三拷贝（对控件新增仍然有效）

1. 第一份拷贝住在 app 里（如 mlaunch `src/ui/`），不抽象不入库；
2. 第二份在 lab/demo 里剥离业务验证；
3. 通过准入清单（第二消费者、fork 缺口说明、不重包、复合优先、文档）才落库。

## 开发

```bash
xmake f -p windows -a x64 --tests=y --demo=y   # 全量（core + duilib + 测试 + demo）
xmake -y && xmake run uikit_core_tests
./build/windows/x64/release/uikit_demo.exe     # 目检窗口：右上角切换深浅主题

xmake f -p linux --core_only=y --tests=y       # 引擎跨平台门禁
./tools/gen_embed.sh                            # 改 design/ 后重新生成内嵌头
```

详细文档见 [docs/](docs/)，重构决策见 [PLAN.md](PLAN.md)。
