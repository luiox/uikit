set_project("taskmgr")
set_version("0.0.1")
set_xmakever("2.8.3")

set_languages("c99")
set_languages("cxx17")

add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})

add_cxflags("/arch:IA32")  -- 如果是 32 位代码
add_cxflags("/O2")        -- 优化级别
add_cxflags("/MT")        -- 静态链接运行时
add_cxflags("/DWIN32")
add_cxflags("/DNDEBUG")
add_cxflags("/D_WINDOWS")
add_cxflags("/DUNICODE")
add_cxflags("/D_UNICODE")

target("taskmgr")
    set_kind("binary")
    -- 强制 32 位，64位下无内联汇编
    set_arch("x86")

    add_includedirs(".")
    add_files("*.cpp")
    add_files("main/*.cpp")
    add_files("taskmgr.rc")
    
    add_defines("WIN32")
    add_defines("NDEBUG")
    add_defines("_WINDOWS")
    add_defines("UNICODE")
    add_defines("_UNICODE")
    -- "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" 
    add_links("kernel32", "user32", "gdi32", "winspool", "comdlg32", "advapi32", "shell32", "ole32", "oleaut32", "uuid", "odbc32", "odbccp32")
