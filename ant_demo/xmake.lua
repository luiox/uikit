set_project("ant_demo")
set_version("0.1.0")
set_xmakever("2.7.9")
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})

-- reuse DuiLib submodule path defined earlier by root project
local duilib_dir = "../third_party/DuiLib_DuiEditor/DuiLib"


-- static library for DuiLib sources (same as root project)
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

-- simple binary
target("ant_demo")
    set_kind("binary")
    set_languages("cxx17")

    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
    else
        set_optimize("faster")
    end

    add_defines("UNICODE", "_UNICODE", "WIN32", "_WINDOWS")
    add_includedirs(duilib_dir, {public = true})
    add_files("src/*.cpp")
    add_deps("DuiLibLite")
    add_syslinks("user32", "gdi32", "comctl32", "comdlg32", "ole32", "oleaut32", "imm32", "winmm", "version", "uxtheme", "shell32")

    after_build(function (target)
        -- copy skin folder to output so resources are found when running
        os.cp(path.join(os.scriptdir(), "skin"), path.join(target:targetdir(), "skin"))
    end)