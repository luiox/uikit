
set_languages("c++17")

target("dll_injector")
    set_kind("binary")
    add_files("src/*.cpp")
    add_includedirs("include", {public = true})
    add_links("kernel32", "user32", "advapi32", "shell32", "psapi")
    add_defines("WIN32_LEAN_AND_MEAN")
    if is_plat("windows") then
        add_defines("DLL_INJECTOR_EXPORTS")
    end
    if is_mode("debug") then
        add_defines("DEBUG")
    end
