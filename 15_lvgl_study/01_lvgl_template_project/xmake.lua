add_rules("mode.debug", "mode.release")

-- 简洁模板（中文注释）
-- 用途：自动检测 Linaro SDK 并启用交叉编译；构建后把产物放到仓库根的 build/artifact/<targetname>
-- 方便复制创建多个子工程（如 01_xxx、03_xxx 等）
local linaro_sdk = "/home/canrad/linux/toolchain/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf"
if os.exists(linaro_sdk) then
    print("检测到 Linaro SDK: " .. linaro_sdk .. "，启用交叉编译")
    set_plat("cross")
    set_arch("arm")
    set_toolchains("arm-linux-gnueabihf")
else
    print("未检测到 Linaro SDK，使用主机编译（x86_64）")
end

-- 目标配置（简单、可复制）
target("01_lvgl_template_project")
    set_kind("binary")
    add_files("src/*.c")

    -- 交叉编译时，强制设置目标动态链接器，以避免板上找不到 INTERP
    if is_plat("cross") or is_arch("arm") then
        add_ldflags("-Wl,--dynamic-linker=/lib/ld-linux-armhf.so.3")
    end

    -- 构建完成后把产物复制到仓库根的 build/artifact/<targetname>
    after_build(function (target)
        -- 假设 xmake 项目位于仓库子目录，repo_root 为上一级目录
        local repo_root = path.join(os.projectdir(), "..")
        local artifact_root = path.join(repo_root, "build", "artifact")
        os.mkdir(artifact_root)
        local target_dir = path.join(artifact_root, target:name())
        os.mkdir(target_dir)
        local out = target:targetfile()
        os.cp(out, target_dir)
        print("[artifact] 已复制到: " .. target_dir)
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

