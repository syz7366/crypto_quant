/**
 * @file test_binance_minimal.cpp
 * @brief 最小化的币安API测试程序
 * @note 参考 python/services/binance_test.py 的设计思路
 * 
 * 目的：用最简单的方式验证币安API是否可以访问
 * 避免复杂的封装，直接使用httplib测试
 */

#include <iostream>
#include <string>

// 启用 httplib 的 OpenSSL 支持
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

// 启用 JSON 解析
#include <json.hpp>

using json = nlohmann::json;

// 是否使用代理（类似Python的proxies参数）
constexpr bool USE_PROXY = true;  // 改为 false 可禁用代理

// 代理服务器地址和端口
constexpr const char* PROXY_HOST = "127.0.0.1";
constexpr int PROXY_PORT = 10090;

/**
 * @brief 测试1：访问币安K线数据（类似Python的test_direct_api）
 */
void test_binance_klines() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "测试：访问币安K线API" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 1. 创建HTTPS客户端
    httplib::SSLClient client("api.binance.com");
    
    // 2. 【关键】禁用SSL证书验证（类似Python的verify=False）
    client.enable_server_certificate_verification(false);
    
    // 3. 【关键】设置代理（类似Python的proxies参数）
    if (USE_PROXY) {
        client.set_proxy(PROXY_HOST, PROXY_PORT);
        std::cout << "✓ 已配置代理: " << PROXY_HOST << ":" << PROXY_PORT << std::endl;
    } else {
        std::cout << "ℹ️  未使用代理（直连）" << std::endl;
    }
    
    // 4. 设置超时（30秒）
    client.set_connection_timeout(30, 0);
    client.set_read_timeout(30, 0);
    
    // 4. 构建请求路径（包含查询参数）
    std::string path = "/api/v3/klines?symbol=BTCUSDT&interval=1h&limit=5";
    
    // 5. 设置HTTP头（模拟浏览器）
    httplib::Headers headers = {
        {"User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7)"},
        {"Accept", "*/*"},
        {"Connection", "keep-alive"}
    };
    
    std::cout << "请求URL: https://api.binance.com" << path << std::endl;
    std::cout << "发送请求中..." << std::endl;
    
    // 6. 发送GET请求
    auto res = client.Get(path, headers);
    
    // 7. 检查结果
    if (!res) {
        // 请求失败（网络层面）
        auto err = res.error();
        std::cerr << "\n❌ 请求失败！" << std::endl;
        std::cerr << "错误代码: " << static_cast<int>(err) << std::endl;
        
        // 打印详细错误信息
        switch (err) {
            case httplib::Error::Connection:
                std::cerr << "错误类型: Connection (连接失败/SSL握手失败)" << std::endl;
                std::cerr << "可能原因: 无法连接到服务器，或SSL握手失败" << std::endl;
                break;
            case httplib::Error::SSLConnection:
                std::cerr << "错误类型: SSL Connection (SSL连接错误)" << std::endl;
                std::cerr << "可能原因: SSL/TLS协议错误" << std::endl;
                break;
            case httplib::Error::SSLServerVerification:
                std::cerr << "错误类型: SSL Server Verification (证书验证失败)" << std::endl;
                std::cerr << "可能原因: 服务器证书无法验证" << std::endl;
                break;
            case httplib::Error::ProxyConnection:
                std::cerr << "错误类型: Proxy Connection (代理连接失败)" << std::endl;
                std::cerr << "可能原因: 无法连接到代理服务器 127.0.0.1:7897" << std::endl;
                std::cerr << "建议: 检查代理服务器是否正在运行" << std::endl;
                break;
            default:
                std::cerr << "错误类型: Unknown (错误代码 " << static_cast<int>(err) << ")" << std::endl;
                if (static_cast<int>(err) == 14) {
                    std::cerr << "注意: 错误代码14通常表示代理连接问题" << std::endl;
                    std::cerr << "建议: 检查代理 127.0.0.1:7897 是否可用" << std::endl;
                }
                break;
        }
        
        std::cerr << "\n建议检查：" << std::endl;
        std::cerr << "1. 网络连接是否正常" << std::endl;
        std::cerr << "2. OpenSSL是否正确安装" << std::endl;
        std::cerr << "3. 是否需要代理" << std::endl;
        return;
    }
    
    // 8. 请求成功，检查HTTP状态码
    std::cout << "\n✅ 连接成功！" << std::endl;
    std::cout << "HTTP状态码: " << res->status << std::endl;
    std::cout << "响应体大小: " << res->body.size() << " bytes" << std::endl;
    
    if (res->status != 200) {
        std::cerr << "\n⚠️  HTTP状态码不是200" << std::endl;
        std::cerr << "响应内容: " << res->body.substr(0, 500) << std::endl;
        return;
    }
    
    // 9. 解析JSON响应
    try {
        auto data = json::parse(res->body);
        
        std::cout << "\n✅ JSON解析成功！" << std::endl;
        std::cout << "返回数据条数: " << data.size() << std::endl;
        
        // 打印前几条K线数据
        std::cout << "\n前3条K线数据：" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        
        for (size_t i = 0; i < std::min(size_t(3), data.size()); i++) {
            auto kline = data[i];
            // K线数据格式: [时间戳, 开, 高, 低, 收, 成交量, ...]
            std::cout << "K线 " << (i+1) << ":" << std::endl;
            std::cout << "  时间戳: " << kline[0] << std::endl;
            std::cout << "  开盘价: " << kline[1] << std::endl;
            std::cout << "  最高价: " << kline[2] << std::endl;
            std::cout << "  最低价: " << kline[3] << std::endl;
            std::cout << "  收盘价: " << kline[4] << std::endl;
            std::cout << "  成交量: " << kline[5] << std::endl;
            std::cout << std::endl;
        }
        
        std::cout << "========================================" << std::endl;
        std::cout << "✅ 测试完全成功！币安API访问正常！" << std::endl;
        std::cout << "========================================" << std::endl;
        
    } catch (const json::exception& e) {
        std::cerr << "\n❌ JSON解析失败: " << e.what() << std::endl;
        std::cerr << "响应内容: " << res->body.substr(0, 500) << std::endl;
    }
}

/**
 * @brief 测试2：访问币安服务器时间（更简单的测试）
 */
void test_binance_time() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "测试：访问币安服务器时间API" << std::endl;
    std::cout << "========================================" << std::endl;
    
    httplib::SSLClient client("api.binance.com");
    client.enable_server_certificate_verification(false);
    
    // 设置代理（与K线测试保持一致）
    if (USE_PROXY) {
        client.set_proxy(PROXY_HOST, PROXY_PORT);
        std::cout << "✓ 已配置代理: " << PROXY_HOST << ":" << PROXY_PORT << std::endl;
    }
    
    client.set_connection_timeout(10, 0);
    
    httplib::Headers headers = {
        {"User-Agent", "Mozilla/5.0"},
        {"Accept", "*/*"}
    };
    
    std::cout << "请求URL: https://api.binance.com/api/v3/time" << std::endl;
    
    auto res = client.Get("/api/v3/time", headers);
    
    if (!res) {
        std::cerr << "❌ 请求失败" << std::endl;
        return;
    }
    
    std::cout << "✅ 状态码: " << res->status << std::endl;
    std::cout << "响应: " << res->body << std::endl;
    
    try {
        auto data = json::parse(res->body);
        long long server_time = data["serverTime"];
        std::cout << "服务器时间戳: " << server_time << std::endl;
    } catch (...) {
        std::cerr << "JSON解析失败" << std::endl;
    }
}

/**
 * @brief 主函数
 */
int main() {
    std::cout << R"(
╔════════════════════════════════════════════════════╗
║     币安API最小化测试程序                          ║
║     Binance API Minimal Test                      ║
╚════════════════════════════════════════════════════╝
)" << std::endl;
    
    std::cout << "类似于 python/services/binance_test.py 的功能" << std::endl;
    std::cout << "使用最简单的方式验证API访问" << std::endl;
    std::cout << "\n当前配置：" << std::endl;
    std::cout << "  代理: " << (USE_PROXY ? "启用" : "禁用");
    if (USE_PROXY) {
        std::cout << " (" << PROXY_HOST << ":" << PROXY_PORT << ")";
    }
    std::cout << std::endl;
    std::cout << "  SSL验证: 禁用（测试模式）" << std::endl;
    
    // 测试1：服务器时间（最简单）
    test_binance_time();
    
    // 测试2：K线数据（完整测试）
    test_binance_klines();
    
    std::cout << "\n所有测试完成！" << std::endl;
    
    return 0;
}

