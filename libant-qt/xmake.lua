set_project("libant-qt")
set_version("0.1.0")
set_xmakever("2.8.2")

add_rules("mode.debug", "mode.release")

includes("third_party/qr-code-generator")
includes("third_party/fast-gaussian-blur")


target("libant_qt")
    set_kind("static")
    set_languages("cxx17")
    add_rules("qt.static")
    add_deps("libicon_qt", "ant_qrcodegen", "ant_fast_gaussian_blur")
    add_frameworks("QtCore", "QtGui", "QtWidgets", "QtSvg")
    add_defines("LIBICON_CORE_ENABLE_EMBED=1", {public = true})
    add_includedirs(
        "src",
        "src/core",
        "src/display",
        "src/feedback",
        "src/input",
        "src/layout",
        "src/navigation",
        "src/overlay",
        "src/view",
        "../libicon-core/generated",
        "../libicon-qt/src",
        {public = true}
    )
    add_headerfiles("src/**.h")
    add_files("src/**.h", {rules = "qt.moc"})
    add_files("src/**.cpp")

    if is_plat("windows") then
        add_syslinks("dwmapi")
    end
