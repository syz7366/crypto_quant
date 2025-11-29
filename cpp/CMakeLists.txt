# =====================================================
# cpp模块的CMakeLists.txt
# 职责：只负责cpp模块的源文件收集和库的构建
# 依赖：所有外部依赖已由根目录CMakeLists.txt配置
# =====================================================

# 添加第三方库的include目录（httplib和json是header-only库）
include_directories(
    ${CMAKE_SOURCE_DIR}/third_party/httplib    
    ${CMAKE_SOURCE_DIR}/third_party/json       
)

# 收集源文件
file(GLOB_RECURSE CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
# 排除测试文件（它们有自己的 main 函数）
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*test_.*\\.cpp$")

# 收集头文件
file(GLOB_RECURSE CORE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)

# 创建核心静态库
add_library(quant_crypto_core_static STATIC ${CORE_SOURCES})

# 设置该库的include目录（PUBLIC：让链接此库的目标也能看到这些头文件）
target_include_directories(quant_crypto_core_static PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接外部依赖（这些依赖由根目录CMakeLists.txt查找和配置）
target_link_libraries(quant_crypto_core_static
    PUBLIC
    Threads::Threads        # 线程库
    OpenSSL::SSL            # OpenSSL库
    OpenSSL::Crypto         # OpenSSL加密库
    ${Boost_LIBRARIES}      # Boost库
)

# 注意：编译选项（-Wall等）已由根目录CMakeLists.txt全局配置，此处无需重复

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

# ==============================================
# 测试可执行文件
# ==============================================

# 测试1：HTTP客户端测试
add_executable(test_http 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/collectors/test_http.cpp
)
target_link_libraries(test_http 
    PRIVATE 
    quant_crypto_core_static
)
set_target_properties(test_http PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 测试2：币安API最小化测试（独立可执行文件，不依赖其他模块）
add_executable(test_binance_minimal 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/collectors/test_binance_minimal.cpp
)
# 只链接必要的库（不需要整个core_static，减少依赖）
target_link_libraries(test_binance_minimal 
    PRIVATE 
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)
set_target_properties(test_binance_minimal PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 测试3： BinanceCollector测试
add_executable(test_binance_collector 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/collectors/test_binance_collector.cpp
)
target_link_libraries(test_binance_collector 
    PRIVATE 
    quant_crypto_core_static
)
set_target_properties(test_binance_collector PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 测试4：数据清洗测试
add_executable(test_data_cleaner
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cleaners/test_data_cleaner.cpp
)
target_link_libraries(test_data_cleaner
    PRIVATE
    quant_crypto_core_static
)
set_target_properties(test_data_cleaner PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

