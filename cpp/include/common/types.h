#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

namespace quant_crypto {

// 基础类型定义
using Timestamp = int64_t;  // Unix毫秒时间戳
using Price = double;
using Volume = double;
using Symbol = std::string;
using Exchange = std::string;

// 时间周期枚举
enum class Timeframe {
    TICK,       // Tick数据
    SECOND_1,   // 1秒
    MINUTE_1,   // 1分钟
    MINUTE_5,   // 5分钟
    MINUTE_15,  // 15分钟
    MINUTE_30,  // 30分钟
    HOUR_1,     // 1小时
    HOUR_4,     // 4小时
    DAY_1,      // 1天
    WEEK_1,     // 1周
    MONTH_1     // 1月
};

// 订单方向
enum class Side {
    BUY,
    SELL,
    UNKNOWN
};

// 订单类型
enum class OrderType {
    LIMIT,
    MARKET,
    STOP_LIMIT,
    STOP_MARKET,
    UNKNOWN
};

// 数据质量标志
enum class DataQuality {
    GOOD,       // 数据质量良好
    SUSPICIOUS, // 数据可疑
    BAD,        // 数据异常
    MISSING     // 数据缺失
};

// OHLCV结构（K线数据）
struct OHLCV {
    Timestamp timestamp;    // 开盘时间戳
    Symbol symbol;          // 交易对
    Exchange exchange;      // 交易所
    Timeframe timeframe;    // 时间周期
    Price open;             // 开盘价
    Price high;             // 最高价
    Price low;              // 最低价
    Price close;            // 收盘价
    Volume volume;          // 成交量
    Volume quote_volume;    // 成交额
    int64_t trades_count;   // 成交笔数
    DataQuality quality;    // 数据质量
    
    OHLCV() : timestamp(0), timeframe(Timeframe::MINUTE_1),
              open(0.0), high(0.0), low(0.0), close(0.0),
              volume(0.0), quote_volume(0.0), trades_count(0),
              quality(DataQuality::GOOD) {}
};

// Tick数据
struct Tick {
    Timestamp timestamp;    // 时间戳
    Symbol symbol;          // 交易对
    Exchange exchange;      // 交易所
    Price price;            // 成交价
    Volume volume;          // 成交量
    Side side;              // 买卖方向
    std::string trade_id;   // 交易ID
    
    Tick() : timestamp(0), price(0.0), volume(0.0), side(Side::UNKNOWN) {}
};

// 订单簿档位
struct OrderBookLevel {
    Price price;            // 价格
    Volume volume;          // 数量
    
    OrderBookLevel() : price(0.0), volume(0.0) {}
    OrderBookLevel(Price p, Volume v) : price(p), volume(v) {}
};

// 订单簿快照
struct OrderBook {
    Timestamp timestamp;                    // 时间戳
    Symbol symbol;                          // 交易对
    Exchange exchange;                      // 交易所
    std::vector<OrderBookLevel> bids;       // 买盘
    std::vector<OrderBookLevel> asks;       // 卖盘
    int64_t sequence;                       // 序列号
    
    OrderBook() : timestamp(0), sequence(0) {}
};

// Ticker数据（24小时行情）
struct Ticker {
    Timestamp timestamp;    // 时间戳
    Symbol symbol;          // 交易对
    Exchange exchange;      // 交易所
    Price last;             // 最新价
    Price bid;              // 买一价
    Price ask;              // 卖一价
    Price high_24h;         // 24小时最高价
    Price low_24h;          // 24小时最低价
    Volume volume_24h;      // 24小时成交量
    Volume quote_volume_24h;// 24小时成交额
    double change_24h;      // 24小时涨跌幅(%)
    
    Ticker() : timestamp(0), last(0.0), bid(0.0), ask(0.0),
               high_24h(0.0), low_24h(0.0), volume_24h(0.0),
               quote_volume_24h(0.0), change_24h(0.0) {}
};

// 时间周期工具函数
inline std::string timeframe_to_string(Timeframe tf) {
    switch (tf) {
        case Timeframe::TICK: return "tick";
        case Timeframe::SECOND_1: return "1s";
        case Timeframe::MINUTE_1: return "1m";
        case Timeframe::MINUTE_5: return "5m";
        case Timeframe::MINUTE_15: return "15m";
        case Timeframe::MINUTE_30: return "30m";
        case Timeframe::HOUR_1: return "1h";
        case Timeframe::HOUR_4: return "4h";
        case Timeframe::DAY_1: return "1d";
        case Timeframe::WEEK_1: return "1w";
        case Timeframe::MONTH_1: return "1M";
        default: return "unknown";
    }
}

inline Timeframe string_to_timeframe(const std::string& str) {
    if (str == "tick") return Timeframe::TICK;
    if (str == "1s") return Timeframe::SECOND_1;
    if (str == "1m") return Timeframe::MINUTE_1;
    if (str == "5m") return Timeframe::MINUTE_5;
    if (str == "15m") return Timeframe::MINUTE_15;
    if (str == "30m") return Timeframe::MINUTE_30;
    if (str == "1h") return Timeframe::HOUR_1;
    if (str == "4h") return Timeframe::HOUR_4;
    if (str == "1d") return Timeframe::DAY_1;
    if (str == "1w") return Timeframe::WEEK_1;
    if (str == "1M") return Timeframe::MONTH_1;
    return Timeframe::MINUTE_1;
}

// 获取时间周期的毫秒数
inline int64_t timeframe_to_milliseconds(Timeframe tf) {
    switch (tf) {
        case Timeframe::SECOND_1: return 1000;
        case Timeframe::MINUTE_1: return 60 * 1000;
        case Timeframe::MINUTE_5: return 5 * 60 * 1000;
        case Timeframe::MINUTE_15: return 15 * 60 * 1000;
        case Timeframe::MINUTE_30: return 30 * 60 * 1000;
        case Timeframe::HOUR_1: return 60 * 60 * 1000;
        case Timeframe::HOUR_4: return 4 * 60 * 60 * 1000;
        case Timeframe::DAY_1: return 24 * 60 * 60 * 1000;
        case Timeframe::WEEK_1: return 7 * 24 * 60 * 60 * 1000;
        default: return 0;
    }
}

enum class ErrorCode{
    SUCCESS = 0,
    NETWORK_ERROR,          // 网络错误
    HTTP_ERROR,             // HTTP 状态码错误
    PARSE_ERROR,            // JSON 解析错误
    INVALID_PARAMS,         // 参数错误
    RATE_LIMIT_EXCEEDED,    // 超过限流
    TIMEOUT,                // 超时
    UNKNOWN_ERROR           // 未知错误
};

// 错误信息-字符串显示
inline std::string error_code_to_string(ErrorCode code){
    switch (code){
        case ErrorCode::SUCCESS: return "Success";
        case ErrorCode::NETWORK_ERROR: return "Network error";
        case ErrorCode::HTTP_ERROR: return "HTTP error";
        case ErrorCode::PARSE_ERROR: return "Parse error";
        case ErrorCode::INVALID_PARAMS: return "Invalid params";
        case ErrorCode::RATE_LIMIT_EXCEEDED: return "Rate limit exceeded";
        case ErrorCode::TIMEOUT: return "Timeout";
        case ErrorCode::UNKNOWN_ERROR: return "Unknown error";
        default: return "Unknown error";
    }
    return "Unknown error and Unknown error code";
}

template<typename T>
struct Result{
    bool success;
    ErrorCode error_code;
    std::string error_message;
    T data;

    Result() : success(false), error_code(ErrorCode::UNKNOWN_ERROR) {}
    
    static Result<T> Ok(T data){
        Result<T> r;
        r.success = true;
        r.error_code = ErrorCode::SUCCESS;
        r.error_message = "";
        r.data = std::move(data);  // 这里可能会有隐患
        return r;
    }

    // 失败时使用
    static Result<T> Err(ErrorCode code, const std::string& message=""){
        Result<T> r;
        r.success = false;
        r.error_code = code;
        r.error_message = message.empty()?error_code_to_string(code):message;
        return r;
    }
};

} // namespace quant_crypto

