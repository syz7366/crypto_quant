#pragma once

#include<string>
#include<vector>
#include "common/types.h"
#include "collectors/http_client.h"
#include "config/config_manager.h"

namespace quant_crypto{
namespace collectors{

using BinanceConfig = quant_crypto::config::BinanceConfig; 

class BinanceCollector {
    public:
        BinanceCollector(const BinanceConfig& config);
        ~BinanceCollector();

        /**
         * @brief 获取K线数据
         * @param symbol 交易对（如 "BTCUSDT"）
         * @param interval 时间周期（如 "1h"）
         * @param limit 数据条数
         * @return OHLCV数据列表
         */
        Result<std::vector<OHLCV>> get_klines(const std::string& symbol, const std::string& interval, int limit = 500);

        /**
         * @brief 获取24小时价格统计
         * @param symbol 交易对
         * @return Ticker数据
         */
        Result<Ticker> get_ticker(const std::string& symbol);

        /**
         * @brief 获取订单簿深度数据
         * @param symbol 交易对
         * @param limit 档位数量
         * @return OrderBook数据
         */
        Result<OrderBook> get_orderbook(const std::string& symbol, int limit = 100);
        // 未来可以添加其他方法
        
    private:
        HttpClient http_client_;    //HTTP 客户端
        std::string base_url_;   // 基础url
        BinanceConfig config_;    // 基础配置
};


}
}

