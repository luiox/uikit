target("crc32-test")
    set_kind("binary")
    add_deps("libca-em")
    add_files("crc32-test.c")

target("pid-test")
    set_kind("binary")
    add_deps("libca-em")
    add_files("pid-test.c")
