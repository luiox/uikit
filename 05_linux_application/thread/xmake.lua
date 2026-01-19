target("thread")
    set_kind("binary")
    add_files("thread.c")
    add_links("pthread")

target("sem")
    set_kind("binary")
    add_files("sem.c")
    add_links("pthread")

target("mutex")
    set_kind("binary")
    add_files("mutex.c")
    add_links("pthread")

target("rwlock")
    set_kind("binary")
    add_files("rwlock.c")
    add_links("pthread")

target("spinlock")
    set_kind("binary")
    add_files("spinlock.c")
    add_links("pthread")

target("lazy_counter")
    set_kind("binary")
    add_files("lazy_counter.c")
    add_links("pthread")
    
target("cond")
    set_kind("binary")
    add_files("cond.c")
    add_links("pthread")

target("thread_pool")
    set_kind("binary")
    add_files("thread_pool.c")
    add_links("pthread")
