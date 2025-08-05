add_rules("mode.debug", "mode.release")

set_languages("c++17") 

add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", { outputdir = "." })
set_encodings("utf-8")

add_requires("opencv 4.10.0")

includes("cv_xmake_template")
includes("find_rect")
