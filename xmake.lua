set_project("nassistant-duilib-launcher")
set_version("0.1.0")
set_xmakever("2.8.3")

add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})
add_requires("nlohmann_json", "gtest")

-- libca 依赖已移除:cpp-duilib-launcher 源码零引用(无 ca:: 符号/头包含),
-- 原 third_party/libca submodule 连同 set_config 门控一并清理。

includes("libicon-core")
includes("libicon-qt")
includes("libant-qt")
includes("libant-qt-demo")
includes("libant-qt-demos")
includes("cpp-duilib-launcher")
includes("ant_demo")
includes("qt6_widget_demo")
