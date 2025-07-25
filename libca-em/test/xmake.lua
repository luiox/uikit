target("crc-test")
    set_kind("binary")
    add_deps("libca-em")
    add_files("crc-test.c")

target("pid-test")
    set_kind("binary")
    add_deps("libca-em")
    add_files("pid-test.c")
