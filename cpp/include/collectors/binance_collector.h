#pragma once

#include<string>
#include<vector>
#include "common/types.h"
#include "common/market_data.h"
#include "collectors/http_client.h"

namespace quant_crypto{
namespace collectors{

class BinanceCollector {

    struct BinanceConfig {
        std::string proxy_host;
        int proxy_port;
        int timeout_ms;
        std::string base_url;
    };

    public:
        BinanceCollector(const BinanceConfig& config);
        ~BinanceCollector();

        // get_klines 方法
        Result<std::vector<Kline>> get_klines(const std::string& symbol, const std::string& interval, int limit = 500);

        // 未来可以添加其他方法
        
    private:
        
        // - HttpClient? 
        // - base_url?
        // - 其他？
};


}
}

