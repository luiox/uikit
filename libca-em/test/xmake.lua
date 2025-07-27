target("crc-test")
    set_kind("binary")
    add_deps("libca-em")
    add_files("crc-test.c")

target("pid-test")
    set_kind("binary")
    add_deps("libca-em")
    add_files("pid-test.c")

target("ping_pong_buffer-test")
    set_kind("binary")
    add_deps("libca-em")
    add_files("ping_pong_buffer-test.c")
