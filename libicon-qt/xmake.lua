set_project("libicon-qt")
set_version("0.1.0")
set_xmakever("2.8.2")

add_rules("mode.debug", "mode.release")

target("libicon_qt")
    set_kind("static")
    set_languages("cxx17")
    set_default(false)
    add_rules("qt.static")

    add_deps("libicon_core_embed")
    add_frameworks("QtCore", "QtGui", "QtSvg")

    add_includedirs("src", {public = true})
    add_headerfiles("src/*.h")
    add_files("src/*.cpp")

    if is_plat("windows") then
        add_defines("UNICODE", "_UNICODE", "NOMINMAX")
    end
