set_project("nassistant-duilib-launcher")
set_version("0.1.0")
set_xmakever("2.7.9")

add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})
add_requires("nlohmann_json", "gtest")

includes("icon-lib")
includes("cpp-duilib-launcher")
