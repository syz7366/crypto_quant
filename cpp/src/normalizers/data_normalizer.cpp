#include "normalizers/data_normalizer.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>

namespace quant_crypto {
namespace normalizers {

// 静态成员初始化
std::unordered_map<std::string, std::unordered_map<std::string, std::string>> 
    DataNormalizer::symbol_mappings_;

std::unordered_map<std::string, std::unordered_map<std::string, std::string>> 
    DataNormalizer::timeframe_mappings_;

std::string DataNormalizer::normalize_symbol(const std::string& symbol, const std::string& exchange) {
    // 简单实现：去除特殊字符，统一格式为 BASE/QUOTE
    std::string normalized = symbol;
    
    // 将下划线和连字符替换为斜杠
    std::replace(normalized.begin(), normalized.end(), '_', '/');
    std::replace(normalized.begin(), normalized.end(), '-', '/');
    
    // 转换为大写
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::toupper);
    
    return normalized;
}

std::string DataNormalizer::denormalize_symbol(const std::string& normalized_symbol, const std::string& exchange) {
    // 反向转换：根据交易所格式
    std::string denormalized = normalized_symbol;
    
    if (exchange == "binance") {
        // Binance使用无分隔符格式：BTCUSDT
        denormalized.erase(std::remove(denormalized.begin(), denormalized.end(), '/'), denormalized.end());
    }
    
    return denormalized;
}

Timestamp DataNormalizer::normalize_timestamp(int64_t timestamp, const std::string& unit) {
    if (unit == "s") {
        return timestamp * 1000;  // 秒转毫秒
    } else if (unit == "us") {
        return timestamp / 1000;  // 微秒转毫秒
    } else if (unit == "ns") {
        return timestamp / 1000000;  // 纳秒转毫秒
    }
    return timestamp;  // 默认已经是毫秒
}

Price DataNormalizer::normalize_price(double price, int precision) {
    double multiplier = std::pow(10.0, precision);
    return std::round(price * multiplier) / multiplier;
}

Volume DataNormalizer::normalize_volume(double volume, int precision) {
    double multiplier = std::pow(10.0, precision);
    return std::round(volume * multiplier) / multiplier;
}

Side DataNormalizer::normalize_side(const std::string& side_str) {
    std::string lower_side = side_str;
    std::transform(lower_side.begin(), lower_side.end(), lower_side.begin(), ::tolower);
    
    if (lower_side == "buy" || lower_side == "bid") {
        return Side::BUY;
    } else if (lower_side == "sell" || lower_side == "ask") {
        return Side::SELL;
    }
    return Side::UNKNOWN;
}

Timeframe DataNormalizer::normalize_timeframe(const std::string& timeframe_str, const std::string& exchange) {
    return string_to_timeframe(timeframe_str);
}

bool DataNormalizer::validate_ohlcv(const OHLCV& ohlcv) {
    // 基本验证
    if (ohlcv.timestamp <= 0) return false;
    if (ohlcv.open <= 0 || ohlcv.high <= 0 || ohlcv.low <= 0 || ohlcv.close <= 0) return false;
    if (ohlcv.volume < 0) return false;
    
    // OHLC关系验证
    if (ohlcv.high < ohlcv.low) return false;
    if (ohlcv.high < ohlcv.open || ohlcv.high < ohlcv.close) return false;
    if (ohlcv.low > ohlcv.open || ohlcv.low > ohlcv.close) return false;
    
    return true;
}

bool DataNormalizer::validate_tick(const Tick& tick) {
    if (tick.timestamp <= 0) return false;
    if (tick.price <= 0) return false;
    if (tick.volume < 0) return false;
    return true;
}

bool DataNormalizer::validate_orderbook(const OrderBook& orderbook) {
    if (orderbook.timestamp <= 0) return false;
    if (orderbook.bids.empty() || orderbook.asks.empty()) return false;
    
    // 检查买盘价格递减
    for (size_t i = 1; i < orderbook.bids.size(); ++i) {
        if (orderbook.bids[i].price > orderbook.bids[i-1].price) return false;
    }
    
    // 检查卖盘价格递增
    for (size_t i = 1; i < orderbook.asks.size(); ++i) {
        if (orderbook.asks[i].price < orderbook.asks[i-1].price) return false;
    }
    
    // 检查买卖价差
    if (!orderbook.bids.empty() && !orderbook.asks.empty()) {
        if (orderbook.bids[0].price >= orderbook.asks[0].price) return false;
    }
    
    return true;
}

} // namespace normalizers
} // namespace quant_crypto

