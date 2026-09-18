-- demo：主题/控件目检窗口（仅 windows，默认不构建，--demo=y 显式开启）
target("uikit_demo")
    set_kind("binary")
    -- GUI 子系统（wWinMain 入口）；本版 xmake 无 set_subsystem，走 ldflags
    add_ldflags("/SUBSYSTEM:WINDOWS", {force = true})
    set_default(false)
    set_languages("cxx17")
    if is_plat("windows") then
        add_cxxflags("/utf-8")
    end
    add_deps("uikit_duilib")
    add_packages("duilib")
    add_files("main.cpp")
