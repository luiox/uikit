set_project("icon-lib")
set_version("0.1.0")
set_xmakever("2.7.9")

add_rules("mode.debug", "mode.release")

task("iconlib_codegen")
    set_menu {
        usage = "xmake iconlib_codegen",
        description = "Manually regenerate icon-lib generated/icons.h and generated/icons.cpp"
    }
    on_run(function ()
        local script = path.join(os.scriptdir(), "scripts", "generate_cpp_assets.py")
        os.exec("python %s", script)
    end)

target("iconlib_dynamic")
    set_kind("static")
    set_languages("cxx17")
    set_default(false)

    add_includedirs("generated", {public = true})
    add_headerfiles("generated/icons.h")
    add_files("generated/icons.cpp")
    add_defines("ICONLIB_ENABLE_EMBED=0", {public = true})

target("iconlib_embed")
    set_kind("static")
    set_languages("cxx17")
    set_default(false)

    add_includedirs("generated", {public = true})
    add_headerfiles("generated/icons.h")
    add_files("generated/icons.cpp")
    add_defines("ICONLIB_ENABLE_EMBED=1", {public = true})
