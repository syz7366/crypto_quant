cmake_minimum_required(VERSION 3.15)

# 收集源文件
file(GLOB_RECURSE CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# 收集头文件
file(GLOB_RECURSE CORE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)

# 创建核心静态库
add_library(quant_crypto_core_static STATIC ${CORE_SOURCES})
target_include_directories(quant_crypto_core_static PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(quant_crypto_core_static
    PUBLIC
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
    ${Boost_LIBRARIES}
)

# 设置编译选项
target_compile_options(quant_crypto_core_static PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Python绑定模块
pybind11_add_module(quant_crypto_core ${CMAKE_CURRENT_SOURCE_DIR}/bindings/bindings.cpp)
target_link_libraries(quant_crypto_core PRIVATE quant_crypto_core_static)
target_include_directories(quant_crypto_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 设置Python模块输出路径
set_target_properties(quant_crypto_core PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/python
)

