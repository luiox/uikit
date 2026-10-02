set_project("uikit")
set_xmakever("2.8.3")
add_rules("mode.debug", "mode.release")

-- 中心包仓：duilib 来自 luiox-repo git 直连包。依赖链自底向上成包，
-- 一个进程只允许一份 DuiLib（全局单例 + ODR），见 PLAN.md「包装设计」。
add_repositories("luiox-repo https://github.com/luiox/luiox-repo.git")

option("core_only")
    set_default(false)
    set_showmenu(true)
    set_description("只构建 L2 主题引擎（不拉 duilib，跨平台）")
option_end()

option("tests")
    set_default(false)
    set_showmenu(true)
    set_description("构建 uikit_core 单元测试")
option_end()

option("demo")
    set_default(false)
    set_showmenu(true)
    set_description("构建主题/控件 demo 窗口（仅 windows）")
option_end()

add_requires("nlohmann_json")
if not has_config("core_only") then
    add_requires("duilib 0.1.0")
end

includes("core")
if not has_config("core_only") then
    includes("duilib")
    if is_plat("windows") and has_config("demo") then
        includes("demo")
    end
end
