set_project("qt6_widget_demo")
set_version("0.1.0")
set_xmakever("2.8.2")

add_rules("mode.debug", "mode.release")

if is_plat("windows") then
    set_runtimes(is_mode("debug") and "MDd" or "MD")
end

target("qt6_widget_demo")
    set_kind("binary")
    set_languages("cxx17")
    add_rules("qt.widgetapp")
    add_frameworks("QtCore", "QtGui", "QtWidgets")
    add_files("src/*.cpp")
    add_headerfiles("src/*.h")

    if is_plat("windows") then
        add_defines("UNICODE", "_UNICODE", "NOMINMAX")
    end
