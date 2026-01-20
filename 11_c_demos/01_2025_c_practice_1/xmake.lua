set_project("2025_c_practice_1")

set_languages("c99")

add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})

add_cflags("/source-charset:utf-8")

target("question1")
    set_kind("binary")
    add_files("src/question1.c")

target("question2")
    set_kind("binary")
    add_files("src/question2.c")

target("question3")
    set_kind("binary")
    add_files("src/question3.c")

