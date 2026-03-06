set_project("libant-qt")
set_version("0.1.0")
set_xmakever("2.8.2")

add_rules("mode.debug", "mode.release")


target("libant_qt")
    set_kind("static")
    set_languages("cxx17")
    add_rules("qt.static")
    add_deps("libicon_qt")
    add_frameworks("QtCore", "QtGui", "QtWidgets", "QtSvg")
    add_defines("LIBICON_CORE_ENABLE_EMBED=1", {public = true})
    add_includedirs(
        ".",
        "ThirdParty/QR-Code-generator-master",
        "ThirdParty/FastGaussianBlur-main",
        "../libicon-core/generated",
        "../libicon-qt/src",
        {public = true}
    )
    add_headerfiles("*.h")
    add_files("*.h", {rules = "qt.moc"})
    add_files("*.cpp")
    add_files("ThirdParty/QR-Code-generator-master/qrcodegen.cpp")

    if is_plat("windows") then
        add_syslinks("dwmapi")
    end
