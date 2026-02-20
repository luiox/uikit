set_project("nassistant-duilib-launcher")
set_version("0.1.0")
set_xmakever("2.7.9")

add_rules("mode.debug", "mode.release")
add_requires("nlohmann_json", "gtest")

target("DuiLibLite")
    set_kind("static")
    set_languages("cxx17")

    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
    else
        set_optimize("faster")
    end

    add_defines("WIN32", "_WIN32", "WINDOWS", "_WIN64", "UNICODE", "_UNICODE", "UILIB_EXPORTS", "UILIB_STATIC")
    add_includedirs("../ref/DuiLib_DuiEditor-master/DuiLib", {public = true})
    add_files("../ref/DuiLib_DuiEditor-master/DuiLib/**.cpp")
    remove_files(
        "../ref/DuiLib_DuiEditor-master/DuiLib/Utils/unzip.cpp",
        "../ref/DuiLib_DuiEditor-master/DuiLib/Utils/UIDataExchange.cpp",
        "../ref/DuiLib_DuiEditor-master/DuiLib/**/**Gtk.cpp",
        "../ref/DuiLib_DuiEditor-master/DuiLib/Render/UIObject_Cairo.cpp",
        "../ref/DuiLib_DuiEditor-master/DuiLib/Render/UIRender_Cairo.cpp",
        "../ref/DuiLib_DuiEditor-master/DuiLib/Render/UIRender_CairoWin32.cpp",
        "../ref/DuiLib_DuiEditor-master/DuiLib/Render/UIRenderFactory_Cairo.cpp"
    )

target("nassistant-duilib")
    set_kind("binary")
    set_languages("cxx17")

    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
    else
        set_optimize("faster")
        add_defines("NASSISTANT_EMBED_SVG_RES=1")
        add_files("res/nassistant.rc")
    end

    add_defines("UNICODE", "_UNICODE", "WIN32", "_WINDOWS")
    add_includedirs("../ref/DuiLib_DuiEditor-master/DuiLib", {public = true})

    add_files("src/*.cpp")
    add_headerfiles("src/*.h")
    add_packages("nlohmann_json")

    add_deps("DuiLibLite")

    add_syslinks("user32", "gdi32", "comctl32", "comdlg32", "ole32", "oleaut32", "imm32", "winmm", "version", "uxtheme", "shell32")

    after_build(function (target)
        os.cp("assets", path.join(target:targetdir(), "assets"))
    end)

target("backend_tests")
    set_kind("binary")
    set_languages("cxx17")

    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
    else
        set_optimize("faster")
    end

    add_defines("UNICODE", "_UNICODE", "WIN32", "_WINDOWS")
    add_includedirs("src", "../ref/DuiLib_DuiEditor-master/DuiLib")
    add_files("src/backend.cpp", "tests/backend_tests.cpp")
    add_packages("nlohmann_json", "gtest")
    add_syslinks("user32", "shell32", "ole32", "oleaut32")
