set_project("nassistant")
set_xmakever("2.8.3")

set_languages("c99")
set_languages("cxx17")

add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", { outputdir = "." })
set_encodings("utf-8")

-- -- qtawesome icon库
-- add_requires("qtawesome")
-- -- qwindowkit 无边框窗口库
-- add_requires("qwindowkit")

target("nassistant")
    add_rules("qt.widgetapp")
    add_files("src/**.cpp")
    -- 因为qt需要编译头文件，所以这里需要添加
    add_files("src/**.h") 

    -- add_packages("qtawesome", "qwindowkit")

