-- L3：duilib 应用器与统一控件。仅 windows（duilib 包本身限定 windows）。
target("uikit_duilib")
    set_kind("static")
    set_languages("cxx17")
    set_warnings("all")
    if is_plat("windows") then
        add_cxxflags("/utf-8")
    end
    add_deps("uikit_core")                  -- 头里引用 theme 类型，公开传导
    add_packages("duilib", {public = true}) -- 定义/包含/链接由包口径统一给出
    add_includedirs("include", {public = true})
    add_files("src/*.cpp")
