#pragma once

#include<boost/beast/core.hpp>
#include<boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <string>
#include <functional>
#include <memory>
#include<atomic>
#include "common/types.h"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

namespace quant_crypto{
namespace ws{

/** 
* @brief 数据回调函数类型
*/
using KlineCallback = std::function<void(const OHLCV&)>;
using TickCallback = std::function<void(const Ticker&)>;
using OrderBookCallback = std::function<void(const OrderBook&)>;


/**
* @class WebSocketClientBase
* @brief WebSocket 客户端抽象基类
 * 职责：
 * 1. 提供通用的WebSocket连接逻辑（Beast实现）
 * 2. 定义统一的订阅接口
 * 3. 子类实现交易所特定的消息解析
*/

class WebSocketClientBase: public std::enable_shared_from_this<WebSocketClientBase>{
public:
    /**
    * @brief 构造函数
    * @param ioc IO上下文
    * @param ctx SSL上下文
    * @param exchange_name 交易所名称（用于日志）
    */
    explicit WebSocketClientBase(
        net::io_context& ioc, 
        ssl::context& ctx, 
        const std::string& exchange_name);
    virtual ~WebSocketClientBase();


    // ========== 通用接口（所有交易所共享） ==========
    
    /**
     * @brief 连接到WebSocket服务器
     */
    void connect(const std::string& host, const std::string& port, const std::string& path);

    /**
    * @brief 断开连接
    */
    void disconnect();

    /**
     * @brief 检查是否已连接
     */
    bool is_connected() const { return connected_; }

    /**
     * @brief 运行IO循环（阻塞）
    */
    void run();

    // ========== 订阅接口（子类必须实现） ==========

    // 订阅K线数据
    virtual void subscribe_kline(const std::string& symbol, const std::string& interval, KlineCallback callback)=0;
    
    // 订阅Ticker数据
    virtual void subscribe_ticker(const std::string& symbol, TickCallback callback)=0;

protected:
    // 子类需要实现的抽象方法
    /** 
    * @brief 解析接收到的消息
    * @param message  JSON消息字符串
    * @note 纯虚函数，每个交易所的消息格式不同
    */
    virtual void parse_message(const std::string& message)=0;


    /**
    * @brief 构建订阅消息
    * @param channel 订阅通道
    * @param symbol 交易对
    * @return 订阅消息的JSON字符串
    *   纯虚函数，每个交易所的订阅格式不一样
    */
    virtual std::string build_subscribe_message(
        const std::string& channel,
        const std::string& symbol
    )=0;

    /** 
    * @brief 发送消息
    */
    void send_message(const std::string& message);

    /**
    * @brief 启动异步读取
    */
    void do_read();
protected:
    //子类可以访问的回调
    KlineCallback kline_callback_;
    TickCallback ticker_callback_;
    OrderBookCallback orderbook_callback_;

    std::string exchange_name_;   // 交易所名称
    
private:
    // 异步操作回调（基类实现）
    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);
    void on_ssl_handshake(beast::error_code ec);
    void on_handshake(beast::error_code ec);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_close(beast::error_code ec);


private:
    tcp::resolver resolver_;
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string path_;

    std::atomic<bool> connected_;
    std::atomic<bool> should_stop_;
};





}
}
