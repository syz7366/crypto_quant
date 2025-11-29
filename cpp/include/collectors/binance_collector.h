#pragma once

#include<string>
#include<vector>
#include "common/types.h"
#include "common/market_data.h"
#include "collectors/http_client.h"

namespace quant_crypto{
namespace collectors{

struct BinanceConfig {
    std::string proxy_host;
    int proxy_port;
    int timeout_ms;
    std::string base_url;
};

class BinanceCollector {
    public:
        BinanceCollector(const BinanceConfig& config);
        ~BinanceCollector();

        // get_klines 方法
        Result<std::vector<common::Kline>> get_klines(const std::string& symbol, const std::string& interval, int limit = 500);

        // 获取24小时价格统计（添加新的方法）
        Result<common::Ticker> get_ticker(const std::string& symbol);

        // 获取订单簿深度数据
        Result<common::OrderBook> get_orderbook(const std::string& symbol, int limit = 100);
        // 未来可以添加其他方法
        
    private:
        HttpClient http_client_;    //HTTP 客户端
        std::string base_url_;   // 基础url
        BinanceConfig config_;    // 基础配置
};


}
}

