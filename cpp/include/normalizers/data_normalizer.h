#pragma once

#include "common/types.h"
#include <string>
#include <unordered_map>

namespace quant_crypto {
namespace normalizers {

/**
 * @brief 数据标准化器
 * 
 * 将不同交易所的数据格式统一化
 */
class DataNormalizer {
public:
    DataNormalizer() = default;
    ~DataNormalizer() = default;

    /**
     * @brief 标准化交易对名称
     * @param symbol 原始交易对
     * @param exchange 交易所名称
     * @return 标准化后的交易对（如 "BTC/USDT"）
     */
    static std::string normalize_symbol(const std::string& symbol, const std::string& exchange);

    /**
     * @brief 反标准化交易对名称
     * @param normalized_symbol 标准交易对
     * @param exchange 交易所名称
     * @return 交易所原始格式的交易对
     */
    static std::string denormalize_symbol(const std::string& normalized_symbol, const std::string& exchange);

    /**
     * @brief 标准化时间戳（统一为UTC毫秒时间戳）
     * @param timestamp 原始时间戳
     * @param unit 时间单位（"s", "ms", "us", "ns"）
     * @return UTC毫秒时间戳
     */
    static Timestamp normalize_timestamp(int64_t timestamp, const std::string& unit = "ms");

    /**
     * @brief 标准化价格精度
     * @param price 原始价格
     * @param precision 精度（小数位数）
     * @return 标准化后的价格
     */
    static Price normalize_price(double price, int precision = 8);

    /**
     * @brief 标准化成交量
     * @param volume 原始成交量
     * @param precision 精度
     * @return 标准化后的成交量
     */
    static Volume normalize_volume(double volume, int precision = 8);

    /**
     * @brief 标准化订单方向
     * @param side_str 原始方向字符串
     * @return Side枚举
     */
    static Side normalize_side(const std::string& side_str);

    /**
     * @brief 标准化时间周期字符串
     * @param timeframe_str 原始时间周期字符串
     * @param exchange 交易所名称
     * @return Timeframe枚举
     */
    static Timeframe normalize_timeframe(const std::string& timeframe_str, const std::string& exchange);

    /**
     * @brief 验证OHLCV数据的基本合理性
     * @param ohlcv OHLCV数据
     * @return 合理返回true，否则返回false
     */
    static bool validate_ohlcv(const OHLCV& ohlcv);

    /**
     * @brief 验证Tick数据的基本合理性
     * @param tick Tick数据
     * @return 合理返回true，否则返回false
     */
    static bool validate_tick(const Tick& tick);

    /**
     * @brief 验证OrderBook数据的基本合理性
     * @param orderbook OrderBook数据
     * @return 合理返回true，否则返回false
     */
    static bool validate_orderbook(const OrderBook& orderbook);

private:
    // 交易对映射表（各交易所格式 -> 标准格式）
    static std::unordered_map<std::string, std::unordered_map<std::string, std::string>> symbol_mappings_;
    
    // 时间周期映射表
    static std::unordered_map<std::string, std::unordered_map<std::string, std::string>> timeframe_mappings_;
};

} // namespace normalizers
} // namespace quant_crypto

