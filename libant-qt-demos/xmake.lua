set_project("libant-qt-demos")
set_version("0.1.0")
set_xmakever("2.8.2")

add_rules("mode.debug", "mode.release")

target("libant_qt_demos")
    set_kind("binary")
    set_languages("cxx17")
    add_rules("qt.widgetapp")
    add_deps("libant_qt")
    add_frameworks("QtCore", "QtGui", "QtWidgets", "QtSvg")
    add_defines("LIBICON_CORE_ENABLE_EMBED=1")
    add_cxflags("/utf-8", {tools = "msvc"})
    add_includedirs(
        ".",
        "src/app",
        "src/pages",
        "src/widgets",
        "../libant-qt/src",
        "../libant-qt/src/core",
        "../libant-qt/src/display",
        "../libant-qt/src/feedback",
        "../libant-qt/src/input",
        "../libant-qt/src/layout",
        "../libant-qt/src/navigation",
        "../libant-qt/src/overlay",
        "../libant-qt/src/view",
        "../libicon-core/generated",
        "../libicon-qt/src"
    )
    add_headerfiles("src/**.h")
    add_files("src/**.h", {rules = "qt.moc"})
    add_files("src/**.cpp")

    if is_plat("windows") then
        add_syslinks("dwmapi")
    end