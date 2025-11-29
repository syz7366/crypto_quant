#pragma once
#include <cstdint>
#include<string>
#include<vector>

namespace quant_crypto {
namespace common {

struct Kline{
    uint64_t open_time;//时间戳
    double open;//开盘价
    double high; // 最高价
    double low;// 最低价
    double close;// 收盘价
    double volume; // 成交量
};

/**
 * @struct Ticker
 * @brief 24小时价格统计数据
 */
struct Ticker{
    std::string symbol;          // 交易对（如 "BTCUSDT"）
    double last_price;           // 最新价格
    double high_price;           // 24小时最高价
    double low_price;            // 24小时最低价
    double volume;               // 24小时成交量
    double price_change_percent; // 24小时涨跌幅(%)

    std::string to_string() const{
        char buffer[256];
        snprintf(buffer, sizeof(buffer),
            "Ticker[%s: price=%.2f, high=%.2f, low=%.2f, volume=%.2f, change=%.2f%%]",
            symbol.c_str(), last_price, high_price, low_price, volume, price_change_percent);
        return std::string(buffer);
    }
};

/**
 * @struct PriceLevel
 * @brief 订单簿价格档位（一个价格和数量）
 */
 struct PriceLevel {
    double price;     // 价格
    double quantity;  // 数量
};

/**
 * @struct OrderBook
 * @brief 订单簿数据（买卖盘深度）
 */
struct OrderBook {
    std::string symbol;                  // 交易对
    std::vector<PriceLevel> bids;       // 买单（买盘）
    std::vector<PriceLevel> asks;       // 卖单（卖盘）
    
    std::string to_string() const {
        char buffer[256];
        snprintf(buffer, sizeof(buffer),
            "OrderBook[%s: bids=%zu levels, asks=%zu levels]",
            symbol.c_str(), bids.size(), asks.size());
        return std::string(buffer);
    }
};

}
}



