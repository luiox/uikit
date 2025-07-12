cmake_minimum_required(VERSION 3.25)

project(cv_proj)

# 添加搜索路径
list(APPEND CMAKE_PREFIX_PATH "D:/sdk/opencv/build/x64/vc15/lib")

# 查找OpenCV
find_package(OpenCV REQUIRED)

# 打印OpenCV版本信息
message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

# 包含OpenCV头文件目录
include_directories(${OpenCV_INCLUDE_DIRS})

# 所有的源代码文件
file(GLOB ALL_CPP_SRCS "src/*.cpp")

# 将源代码添加到此项目的可执行文件。
add_executable (OpenCvProject ${ALL_CPP_SRCS})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET OpenCvProject PROPERTY CXX_STANDARD 20)
endif()

# 链接OpenCV库到项目
target_link_libraries(OpenCvProject ${OpenCV_LIBS})