set_project("nassistant-duilib-launcher")
set_version("0.1.0")
set_xmakever("2.8.3")

add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})
add_requires("nlohmann_json", "gtest")

-- libca options: only enable C++ core, disable embedded MCU and demo
set_config("with_core", true)
set_config("with_em", false)
set_config("with_demo", false)
set_config("with_tests", false)

includes("third_party/libca")
includes("libicon-core")
includes("libicon-qt")
includes("libant-qt")
includes("libant-qt-demo")
includes("libant-qt-demos")
includes("cpp-duilib-launcher")
includes("ant_demo")
includes("qt6_widget_demo")
