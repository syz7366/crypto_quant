#include "websocket/websocket_factory.h"
#include "websocket/binance_websocket_client.h"
#include <iostream>
#include <algorithm>


namespace quant_crypto{
namespace ws{
std::shared_ptr<WebSocketClientBase> WebSocketFactory::create(
    const std::string& exchange,
    net::io_context& ioc,
    ssl::context& ctx
){
    std::string lower_exchange = exchange;
    std::transform(lower_exchange.begin(), lower_exchange.end(), 
                   lower_exchange.begin(), ::tolower);
    
    if (lower_exchange == "binance") {
        std::cout << "[Factory] 创建 Binance WebSocket 客户端" << std::endl;
        return std::make_shared<BinanceWebSocketClient>(ioc, ctx);
    }
    // 这里未来可以添加更多交易所

    else{
        std::cerr << "[Factory] 不支持的交易所: " << exchange << std::endl;
        return nullptr;
    }
}




}
}



