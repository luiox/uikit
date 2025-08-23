target("anonymous_pipe")
    set_kind("binary")
    add_files("anonymous_pipe.c")

target("named_pipe")
    set_kind("binary")
    add_files("named_pipe.c")

target("signal")
    set_kind("binary")
    add_defines("_GNU_SOURCE")
    add_files("signal.c")

target("msga")
    set_kind("binary")
    add_files("msga.c")

target("msgb")
    set_kind("binary")
    add_files("msgb.c")

target("shma")
    set_kind("binary")
    add_files("shma.c")

target("shmb")
    set_kind("binary")
    add_files("shmb.c")

target("ipc_sem")
    set_kind("binary")
    add_files("sem.c")
