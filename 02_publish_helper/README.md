发布辅助工具

目前设计为辅助C++的构建后的打包工作

使用toml进行配置，使用`mpub_package.toml`这样子的文件来描述如何打包和发布这个包。

```toml
# mpub_package.toml
[package]
name = "libA"
version = "1.2.3"
authors = ["teamX <contact@example.com>"]
license = "MIT"
description = "JNI bridge library"

[build]
type = "shared"  # shared/static/both
compiler = "gcc"
flags = ["-O3", "-fPIC"]
cxxflags = ["-std=c++17"]
ldflags = ["-lstdc++"]

[dependencies]
libB = { local = "libs/libB" }
openssl = { conan = "openssl/1.1.1" }
git_deps = [
    { url = "https://github.com/...", tag = "v1.0" }
]

[headers]
includedir = "include"
sources = [
    "src/include/**/*.h",
    "src/include/**/*.hpp"
]

[platforms]
linux = { arch = ["x86_64", "aarch64"] }
windows = { arch = ["x86", "x64"] }

[metadata]
repository = "https://github.com/teamX/libA"
homepage = "https://example.com/libA"
keywords = ["jni", "interop"]

[plugins]
signing = { key = "dev.p12" }
docs = { format = "doxygen" }

```

我觉得可以就是对于需要特殊处理的，直接在toml里面指定一个python脚本，然后提供一个接口给他hook来修改发布过程。




















发布库，先用python调用xmake构建，然后写python脚本收集构建的产物，完事了再把头文件收好，基本上就是一个可以发布的二进制了
