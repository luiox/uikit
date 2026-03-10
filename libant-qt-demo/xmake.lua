set_project("libant-qt-demo")
set_version("0.1.0")
set_xmakever("2.8.2")

add_rules("mode.debug", "mode.release")


target("libant_qt_demo")
    set_kind("binary")
    set_languages("cxx17")
    add_rules("qt.widgetapp")
    add_deps("libant_qt")
    add_frameworks("QtCore", "QtGui", "QtWidgets", "QtSvg")
    add_defines("LIBICON_CORE_ENABLE_EMBED=1")
    add_includedirs(
        ".",
        "src/app",
        "src/pages",
        "src/showcase",
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
    add_files(
        "src/**.cpp",
        "LibAntQtDemoWindow.ui",
        "assets/libant-qt-demo.qrc"
    )

    if is_plat("windows") then
        add_syslinks("dwmapi")
    end
