cmake_minimum_required(VERSION 3.15)
project(quant_crypto_data VERSION 1.0.0 LANGUAGES CXX)

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
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# Boost - 使用header-only模式（暂时不依赖编译的库）
find_package(Boost REQUIRED)
if(Boost_FOUND)
    message(STATUS "Found Boost ${Boost_VERSION}")
endif()

# pybind11 - 优先使用系统安装的版本
find_package(pybind11 CONFIG)
if(NOT pybind11_FOUND)
    # 如果系统没有安装，使用third_party目录下的
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/pybind11)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/pybind11 EXCLUDE_FROM_ALL)
    else()
        message(FATAL_ERROR "pybind11 not found. Please run: pip install pybind11")
    endif()
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

