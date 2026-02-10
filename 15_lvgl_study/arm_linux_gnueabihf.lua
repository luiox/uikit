
-- 定义imx6ull的交叉编译工具链
toolchain("arm-linux-gnueabihf")
    set_kind("standalone")
    local sdk_root = "/home/canrad/linux/toolchain/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf"
    local bin_dir = path.join(sdk_root, "bin")
   
    set_sdkdir(sdk_root)
    set_bindir(bin_dir)

    local sysroot = path.join(sdk_root, "arm-linux-gnueabihf", "libc")
    add_cxflags("--sysroot=" .. sysroot, {force = true})
    add_ldflags("--sysroot=" .. sysroot, {force = true})
        
    -- 2. 显式链接 C 库和数学库
    add_syslinks("c", "m", "pthread")

    -- 设置工具集映射
    set_toolset("cc", "arm-linux-gnueabihf-gcc")
    set_toolset("cxx", "arm-linux-gnueabihf-g++")
    set_toolset("as", "arm-linux-gnueabihf-as")
    set_toolset("ld", "arm-linux-gnueabihf-ld")
    set_toolset("objcopy", "arm-linux-gnueabihf-objcopy")
    set_toolset("size", "arm-linux-gnueabihf-size")
toolchain_end()

