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
        "../libant-qt/src",
        "../libicon-core/generated",
        "../libicon-qt/src"
    )
    add_headerfiles("*.h")
    add_files(
        "LibAntQtDemoWindow.h",
        "HomePage.h",
        "FunctionPage.h",
        "SettingsPage.h",
        "AboutPage.h",
        {rules = "qt.moc"}
    )
    add_files(
        "main.cpp",
        "LibAntQtDemoWindow.cpp",
        "HomePage.cpp",
        "FunctionPage.cpp",
        "SettingsPage.cpp",
        "AboutPage.cpp",
        "LibAntQtDemoWindow.ui",
        "assets/libant-qt-demo.qrc"
    )

    if is_plat("windows") then
        add_syslinks("dwmapi")
    end
