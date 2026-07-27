-- directory where the DuiLib source submodule is checked out
local duilib_dir = "../third_party/DuiLib_DuiEditor/DuiLib"

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
    add_includedirs(duilib_dir, {public = true})
    add_files(duilib_dir .. "/**.cpp")
    remove_files(
        duilib_dir .. "/Utils/unzip.cpp",
        duilib_dir .. "/Utils/UIDataExchange.cpp",
        duilib_dir .. "/**/**Gtk.cpp",
        duilib_dir .. "/Render/UIObject_Cairo.cpp",
        duilib_dir .. "/Render/UIRender_Cairo.cpp",
        duilib_dir .. "/Render/UIRender_CairoWin32.cpp",
        duilib_dir .. "/Render/UIRenderFactory_Cairo.cpp"
    )

target("nassistant-duilib")
    set_kind("binary")
    set_languages("cxx17")

    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
        add_deps("libicon_core_dynamic")
    else
        set_optimize("faster")
        add_deps("libicon_core_embed")
    end

    add_defines("UNICODE", "_UNICODE", "WIN32", "_WINDOWS")
    add_includedirs(duilib_dir, {public = true})

    add_files("src/*.cpp", "src/utils/*.cpp")
    add_headerfiles("src/*.h")
    add_packages("nlohmann_json")
    add_deps("libca_json")
    add_includedirs("../third_party/libca/libca/src", {public = true})

    add_deps("DuiLibLite")

    add_syslinks("user32", "gdi32", "comctl32", "comdlg32", "ole32", "oleaut32", "imm32", "winmm", "version", "uxtheme", "shell32")

    after_build(function (target)
        if is_mode("debug") then
            os.cp(path.join(os.scriptdir(), "..", "libicon-core", "icons"), path.join(target:targetdir(), "icons"))
        end
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
    add_includedirs("src", duilib_dir)
    add_files("src/backend.cpp", "src/logger.cpp", "src/utils/*.cpp", "tests/backend_tests.cpp")
    add_packages("nlohmann_json", "gtest")
    add_syslinks("user32", "shell32", "ole32", "oleaut32")
