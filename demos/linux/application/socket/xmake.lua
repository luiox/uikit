target("icmp_test")
    set_kind("binary")
    add_files("icmp_test.c")

target("httpserver")
    set_kind("binary")
    add_files("httpserver.c")
    add_links("pthread")
