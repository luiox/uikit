-- 简洁模板（中文注释）
-- 用途：自动检测 Linaro SDK 并启用交叉编译；集成 lvgl 子模块并提供 framebuffer 驱动示例
local linaro_sdk = "/home/canrad/linux/toolchain/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf"
if os.exists(linaro_sdk) then
    print("检测到 Linaro SDK: " .. linaro_sdk .. "，启用交叉编译")
    set_plat("cross")
    set_arch("arm")
    -- 直接设置工具链二进制路径，避免依赖全局 toolchain 注册
    local bin_dir = path.join(linaro_sdk, "bin")
    set_toolset("cc", path.join(bin_dir, "arm-linux-gnueabihf-gcc"))
    set_toolset("cxx", path.join(bin_dir, "arm-linux-gnueabihf-g++"))
    set_toolset("as", path.join(bin_dir, "arm-linux-gnueabihf-as"))
    set_toolset("ld", path.join(bin_dir, "arm-linux-gnueabihf-gcc"))
    -- 指定 sysroot
    add_cxflags("--sysroot=" .. path.join(linaro_sdk, "arm-linux-gnueabihf", "libc"), {force = true})
    add_ldflags("--sysroot=" .. path.join(linaro_sdk, "arm-linux-gnueabihf", "libc"), {force = true})
else
    print("未检测到 Linaro SDK，使用主机编译（x86_64）")
end

-- 如果存在 lvgl 子模块，则编译为静态库并暴露给主程序
if os.exists(path.join(os.projectdir(), "..", "lvgl")) then
    target("lvgl")
        set_kind("static")
        -- LVGL 代码使用 C99 语法（for(int i=...))，设置为 C99 标准
        set_languages("c99")
        add_includedirs("../lvgl", "include")
        add_defines("LV_CONF_INCLUDE_SIMPLE", "LV_USE_LINUX_FBDEV=1", "LV_LINUX_FBDEV_MMAP=1", "_POSIX_C_SOURCE=199309L")
        add_files("../lvgl/src/**.c")
        set_warnings("all", "no")
end

-- 主程序
target("01_lvgl_template_project")
    set_kind("binary")

    -- 指定生成目录为项目内的 build/arm，确保构建产物可预测
    set_targetdir(path.join(os.projectdir(), "build", "arm"))

    -- 包含 LVGL 和本地 include
    add_includedirs("include", "../lvgl")
    -- 启用 LVGL 的 fbdev 驱动头声明
    add_defines("LV_USE_LINUX_FBDEV=1")

    -- 链接 lvgl 库（如果已编译）
    if os.exists(path.join(os.projectdir(), "..", "lvgl")) then
        add_deps("lvgl")
    end

    -- 需要 pthread 支持（用于 tick 线程）
    add_syslinks("pthread", "rt")

    -- 源文件：保留原有 hello main，并加入 lvgl demo 与 framebuffer 驱动
    add_files("src/*.c", "src/fbdev_drv.c", "src/lvgl_demo.c")

    -- 交叉编译时，强制设置目标动态链接器，以避免板上找不到 INTERP
    if is_plat("cross") or is_arch("arm") then
        add_ldflags("-Wl,--dynamic-linker=/lib/ld-linux-armhf.so.3")
    end

    -- 构建后把产物复制到仓库根的 build/artifact/<target>
    after_build(function (target)
        -- 将产物放到工作区根的 build/artifact/<targetname>
        local workspace_root = path.join(os.projectdir(), "..", "..")
        local artifact_root = path.join(workspace_root, "build", "artifact")
        os.mkdir(artifact_root)
        local target_dir = path.join(artifact_root, target:name())
        os.mkdir(target_dir)
        -- 优先选择 build/arm/ 产物（交叉构建），否则回退到 target:targetfile()
        -- 构建输出通常位于上层项目的 build 目录（如 15_lvgl_study/build/arm/...）
        local candidate = path.join(os.projectdir(), "..", "build", "arm", target:name())
        if not os.isfile(candidate) then
            candidate = target:targetfile()
        end
        if os.isfile(candidate) then
            os.cp(candidate, target_dir)
            print("[artifact] 已复制: " .. candidate .. " -> " .. target_dir)
        else
            print("[artifact] 未找到要复制的二进制：" .. candidate)
        end
    end)


--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro definition
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--

