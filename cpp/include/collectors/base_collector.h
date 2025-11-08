#pragma once

#include "common/types.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace quant_crypto {
namespace collectors {

// 数据回调函数类型
using OHLCVCallback = std::function<void(const OHLCV&)>;
using TickCallback = std::function<void(const Tick&)>;
using OrderBookCallback = std::function<void(const OrderBook&)>;
using TickerCallback = std::function<void(const Ticker&)>;

/**
 * @brief 数据采集器基类
 * 
 * 定义了所有交易所数据采集器的统一接口
 */
class BaseCollector {
public:
    virtual ~BaseCollector() = default;

    /**
     * @brief 初始化采集器
     * @return 成功返回true，失败返回false
     */
    virtual bool initialize() = 0;

    /**
     * @brief 启动采集器
     * @return 成功返回true，失败返回false
     */
    virtual bool start() = 0;

    /**
     * @brief 停止采集器
     */
    virtual void stop() = 0;

    /**
     * @brief 检查采集器是否正在运行
     * @return 正在运行返回true，否则返回false
     */
    virtual bool is_running() const = 0;

    /**
     * @brief 订阅K线数据
     * @param symbol 交易对
     * @param timeframe 时间周期
     * @param callback 数据回调函数
     * @return 成功返回true，失败返回false
     */
    virtual bool subscribe_ohlcv(
        const Symbol& symbol,
        Timeframe timeframe,
        OHLCVCallback callback) = 0;

    /**
     * @brief 订阅Tick数据
     * @param symbol 交易对
     * @param callback 数据回调函数
     * @return 成功返回true，失败返回false
     */
    virtual bool subscribe_tick(
        const Symbol& symbol,
        TickCallback callback) = 0;

    /**
     * @brief 订阅订单簿数据
     * @param symbol 交易对
     * @param depth 深度档位数
     * @param callback 数据回调函数
     * @return 成功返回true，失败返回false
     */
    virtual bool subscribe_orderbook(
        const Symbol& symbol,
        int depth,
        OrderBookCallback callback) = 0;

    /**
     * @brief 订阅Ticker数据
     * @param symbol 交易对
     * @param callback 数据回调函数
     * @return 成功返回true，失败返回false
     */
    virtual bool subscribe_ticker(
        const Symbol& symbol,
        TickerCallback callback) = 0;

    /**
     * @brief 取消订阅K线数据
     * @param symbol 交易对
     * @param timeframe 时间周期
     */
    virtual void unsubscribe_ohlcv(
        const Symbol& symbol,
        Timeframe timeframe) = 0;

    /**
     * @brief 取消订阅Tick数据
     * @param symbol 交易对
     */
    virtual void unsubscribe_tick(const Symbol& symbol) = 0;

    /**
     * @brief 取消订阅订单簿数据
     * @param symbol 交易对
     */
    virtual void unsubscribe_orderbook(const Symbol& symbol) = 0;

    /**
     * @brief 取消订阅Ticker数据
     * @param symbol 交易对
     */
    virtual void unsubscribe_ticker(const Symbol& symbol) = 0;

    /**
     * @brief 获取历史K线数据（REST API）
     * @param symbol 交易对
     * @param timeframe 时间周期
     * @param start_time 开始时间戳
     * @param end_time 结束时间戳
     * @param limit 最大返回数量
     * @return K线数据列表
     */
    virtual std::vector<OHLCV> fetch_ohlcv(
        const Symbol& symbol,
        Timeframe timeframe,
        Timestamp start_time,
        Timestamp end_time,
        int limit = 1000) = 0;

    /**
     * @brief 获取当前订单簿快照（REST API）
     * @param symbol 交易对
     * @param depth 深度档位数
     * @return 订单簿快照
     */
    virtual OrderBook fetch_orderbook(
        const Symbol& symbol,
        int depth = 20) = 0;

    /**
     * @brief 获取当前Ticker（REST API）
     * @param symbol 交易对
     * @return Ticker数据
     */
    virtual Ticker fetch_ticker(const Symbol& symbol) = 0;

    /**
     * @brief 获取交易所名称
     * @return 交易所名称
     */
    virtual std::string get_exchange_name() const = 0;

    /**
     * @brief 获取支持的交易对列表
     * @return 交易对列表
     */
    virtual std::vector<Symbol> get_symbols() const = 0;

    /**
     * @brief 测试连接
     * @return 连接正常返回true，否则返回false
     */
    virtual bool test_connection() = 0;

protected:
    std::string exchange_name_;
    bool is_running_ = false;
};

} // namespace collectors
} // namespace quant_crypto

