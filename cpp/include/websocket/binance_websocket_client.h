#pragma once

// 兼容新版Boost的宏定义  
#define BOOST_ASIO_HAS_STD_CHRONO
#define BOOST_ASIO_DISABLE_STD_CHRONO
#define BOOST_ASIO_USE_TS_EXECUTOR_AS_DEFAULT

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <json.hpp>
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include "common/types.h"

namespace quant_crypto{
namespace websocket{

using WebsocketClient = websocketpp::client<websocketpp::config::asio_client>;
using WebsocketMessagePtr = websocketpp::config::asio_client::message_type::ptr;
using WebsocketContext = websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>;


/**
 * @brief K线WebSocket数据回调函数
 * @param ohlcv K线数据
 */
using KlineCallback = std::function<void(const OHLCV&)>;
/**
 * @class BinanceWebSocketClient
 * @brief 币安WebSocket客户端
 * 
 * 功能：
 * - 订阅实时K线数据
 * - 自动解析JSON消息
 * - 回调通知
 */
class BinanceWebSocketClient{
public:

    /**
     * @brief 构造函数
     */
    BinanceWebSocketClient();

    /**
     * @brief 析构函数
     */
    ~BinanceWebSocketClient();

    /**
     * @brief 订阅K线数据流
     * @param symbol 交易对（如 "btcusdt"，注意小写）
     * @param interval 时间周期（如 "1m", "1h"）
     * @param callback 数据回调函数
     * @return 是否订阅成功
     */
     bool subscribe_kline(const std::string& symbol, 
        const std::string& interval,
        KlineCallback callback);

    /**
     * @brief 停止WebSocket连接
     */
     void stop();

    /**
     * @brief 检查是否已连接
     */
     bool is_connected() const { return connected_.load(); }

private:
    /**
     * @brief 连接到WebSocket
     */
     bool connect(const std::string& uri);

    /**
     * @brief 消息处理函数
     */
    void on_message(websocketpp::connection_hdl hdl, WebsocketMessagePtr msg);
    
    /**
     * @brief 连接打开回调
     */
    void on_open(websocketpp::connection_hdl hdl);

    /**
     * @brief 连接关闭回调
     */
    void on_close(websocketpp::connection_hdl hdl);

    /**
     * @brief 连接失败回调
     */
    void on_fail(websocketpp::connection_hdl hdl);

    /**
     * @brief SSL初始化
     */
    WebsocketContext on_tls_init(websocketpp::connection_hdl hdl);
    
     /**
      * @brief 运行IO循环（在独立线程中）
      */
    void run();

private:
    WebsocketClient client_;                // WebSocket客户端
    websocketpp::connection_hdl hdl_;       // 连接句柄
    std::thread io_thread_;                 // IO线程
    std::atomic<bool> connected_;           // 连接状态
    std::atomic<bool> should_stop_;         // 停止标志

    KlineCallback kline_callback_;          // K线数据回调
    std::string subscribed_symbol_;         // 订阅的交易对
    std::string subscribed_interval_;       // 订阅的周期

};


}
}






