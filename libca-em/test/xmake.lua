target("test-crc")
    set_kind("binary")
    add_deps("libca-em")
    add_files("crc-test.c")

target("test-pid")
    set_kind("binary")
    add_deps("libca-em")
    add_files("pid-test.c")

target("test-ping_pong_buffer")
    set_kind("binary")
    add_deps("libca-em")
    add_files("ping_pong_buffer-test.c")
