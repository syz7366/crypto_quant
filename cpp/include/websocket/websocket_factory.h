#pragma once
#include "websocket/websocket_client_base.h"
#include <memory>

namespace quant_crypto{
namespace ws {

/**
* @brief WebSocket 客户端工厂
*/

class WebSocketFactory{
public:
    /** @brief 创建WebSocket客户端
    * @param exchange 交易所名称 （binance，okx，bybit）
    */
    static std::shared_ptr<WebSocketClientBase> create(
        const std::string& exchange,
        net::io_context& ios,
        ssl::context& ctx
    );
};



}
}

