#include <gtest/gtest.h>

// 自带入口，不依赖 gtest 包的 main 配置（xmake gtest main=true 各版本行为有差异）。
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
