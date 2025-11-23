cmake_minimum_required(VERSION 3.15)
project(quant_crypto_data VERSION 1.0.0 LANGUAGES CXX)

# 生成 compile_commands.json 供 IDE 使用（用于代码跳转、补全等）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置CMake策略以消除警告
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)  # 使用新的FindBoost模块
endif()
if(POLICY CMP0148)
    cmake_policy(SET CMP0148 NEW)  # 使用新的Python查找模块
endif()

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -march=native)
    endif()
endif()

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 查找依赖
message(STATUS "========================================")
message(STATUS "正在查找项目依赖...")
message(STATUS "========================================")

find_package(Threads REQUIRED)
message(STATUS "✓ 找到 Threads")

find_package(OpenSSL REQUIRED)
message(STATUS "✓ 找到 OpenSSL ${OPENSSL_VERSION}")

# Boost - 使用header-only模式（暂时不依赖编译的库）
# 使用CONFIG模式查找Boost（推荐的新方式）
find_package(Boost 1.70 CONFIG QUIET)
if(NOT Boost_FOUND)
    # 如果CONFIG模式失败，回退到Module模式
    find_package(Boost 1.70 REQUIRED)
endif()
if(Boost_FOUND)
    message(STATUS "✓ 找到 Boost ${Boost_VERSION}")
endif()

# pybind11 - 优先使用系统安装的版本
find_package(pybind11 CONFIG QUIET)
if(NOT pybind11_FOUND)
    # 如果系统没有安装，使用third_party目录下的
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/pybind11)
        message(STATUS "使用项目内置的 pybind11")
        # 设置pybind11子项目的策略，抑制其警告
        set(PYBIND11_INSTALL OFF CACHE BOOL "")
        set(PYBIND11_TEST OFF CACHE BOOL "")
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/pybind11 EXCLUDE_FROM_ALL)
        message(STATUS "✓ 找到 pybind11 (third_party)")
    else()
        message(FATAL_ERROR "❌ 找不到 pybind11。请运行: pip install pybind11")
    endif()
else()
    message(STATUS "✓ 找到 pybind11 (系统安装)")
endif()

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/include
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

# 添加子目录
add_subdirectory(cpp)

# 安装规则
# install(TARGETS quant_crypto_core
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
# )

# 测试 (暂时禁用，等实现测试后再启用)
# enable_testing()
# add_subdirectory(cpp/tests)

