
-- 自动更新 VSCode 的智能提示配置
add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})
-- 如果上面这个不行使用下面这个命令
-- xmake project -k compile_commands

includes("arm_linux_gnueabihf.lua")

includes("01_lvgl_template_project")
