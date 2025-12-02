#pragma once
#include "websocket/websocket_client_base.h"
#include <json.hpp>

namespace quant_crypto{
namespace ws{ // 使用ws命名空间避免与标准库冲突

/**
* @class BinanceWebSocketClient
* @brief 币安WebSocket客户端实现
* 
* 特点：
* - 继承通用WebSocket逻辑
* - 实现币安特定的消息格式解析
*/
class BinanceWebSocketClient: public WebSocketClientBase{

public:
    // 构造函数     
    explicit BinanceWebSocketClient(net::io_context& ioc, ssl::context& ctx);

    // 订阅K线数据（币安实现）
    void subscribe_kline(const std::string& symbol, const std::string& interval, KlineCallback callback) override;
    // 订阅Ticker数据（币安实现）
    void subscribe_ticker(const std::string& symbol, TickCallback callback) override;

protected:
    // 解析接收到的消息（币安实现）
    void parse_message(const std::string& message) override;

    // 构建订阅消息（币安实现）
    std::string build_subscribe_message(
        const std::string& channel, 
        const std::string& symbol
    ) override;

private:
    // 解析具体的K线消息
    void parse_kline_message(const nlohmann::json& j);
    // 解析具体的Ticker消息
    void parse_ticker_message(const nlohmann::json& j);

};

}
}






