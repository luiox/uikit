
-- 自动更新 VSCode 的智能提示配置
add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})
-- 如果上面这个不行使用下面这个命令
-- xmake project -k compile_commands

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

includes("arm_linux_gnueabihf.lua")

includes("01_lvgl_template_project")
