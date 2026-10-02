-- L2 主题引擎：纯 C++17。此目录禁止 include duilib/Qt 任何头——框架无关性
-- 由构建边界钉死（CI 在 linux 上以 core_only 形态构建并跑测，作为门禁）。

target("uikit_core")
    set_kind("static")
    set_languages("cxx17")
    set_warnings("all")
    add_packages("nlohmann_json")
    if is_plat("windows") then
        -- 源码含中文注释，MSVC 需显式声明输入编码
        add_cxxflags("/utf-8")
    end
    add_includedirs("include", {public = true})
    -- 公开头随 xmake install 进 installdir/include（包定义零手动拷贝）；
    -- 括号语法以 uikit/ 为安装根，保持 "uikit/theme/..." 引用结构。
    add_headerfiles("include/(uikit/**)")
    add_files("src/*.cpp")
target_end()

-- 单测与引擎同仓：规则简单自定（PLAN.md 阶段三第 1 条），不套 libca 模块规范。
if has_config("tests") then
    add_requires("gtest")

    target("uikit_core_tests")
        set_kind("binary")
        set_default(false)
        set_languages("cxx17")
        add_deps("uikit_core")
        add_packages("gtest")
        if is_plat("windows") then
            add_cxxflags("/utf-8")
        end
        -- 自带入口（unittest/main.cpp），不依赖 gtest 包的 main 配置
        add_files("unittest/main.cpp", "unittest/test_*.cpp")
end
