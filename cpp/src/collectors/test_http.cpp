#include "collectors/http_client.h"
#include <iostream>

int main() {
    using namespace quant_crypto::collectors;
    
    std::cout << "========================================" << std::endl;
    std::cout << "测试 HttpClient 类（使用代理）" << std::endl;
    std::cout << "========================================" << std::endl;
    
    HttpClient client;
    client.set_timeout(30000);  // 30秒
    
    // 【关键】设置代理（对应Python的proxies参数）
    client.set_proxy("127.0.0.1", 10090);
    std::cout << "✓ 已配置代理: 127.0.0.1:10090" << std::endl;
    std::cout << std::endl;
    
    // 测试获取 Binance K线数据
    std::map<std::string, std::string> params;
    params["symbol"] = "BTCUSDT";
    params["interval"] = "1h";
    params["limit"] = "5";
    
    std::string url = "https://api.binance.com/api/v3/klines";
    std::cout << "Testing URL: [" << url << "]" << std::endl;
    std::cout << "URL length: " << url.length() << std::endl;
    auto result = client.get(url, params);
    
    if (result.success) {
        std::cout << "Success!" << std::endl;
        std::cout << "Status: " << result.data.status_code << std::endl;
        std::cout << "Body length: " << result.data.body.length() << std::endl;
        std::cout << "Body preview: " << result.data.body.substr(0, 200) << "..." << std::endl;
    } else {
        std::cerr << "Failed: " << result.error_message << std::endl;
    }
    
    return 0;
}