#include "websocket/binance_websocket_client.h"
#include <iostream>
#include <sstream>
#include <algorithm>


namespace quant_crypto{
namespace ws{

// =============== 构造函数 =================
BinanceWebSocketClient::BinanceWebSocketClient(net::io_context& ioc, ssl::context& ctx)
    : WebSocketClientBase(ioc, ctx, "Binance")
{
    std::cout << "[BinanceWebSocketClient] 初始化完成" << std::endl;
}

// ================ 订阅接口实现 ===============
void BinanceWebSocketClient::subscribe_kline(
    const std::string& symbol, 
    const std::string& interval, 
    KlineCallback callback){
    
        kline_callback_ = callback;
        // 转换为小写 （API调用要求的）
        std::string lower_symbol = symbol;
        std::transform(lower_symbol.begin(), lower_symbol.end(),
                        lower_symbol.begin(), ::tolower);
        
        // 币安 K线WebSocket 路径 /ws/btcusdt@kline_1m
        std::string path = "/ws/" + lower_symbol + "@kline_" + interval;
        std::cout << "[Binance] 订阅K线: " << symbol << " @ " << interval << std::endl;

        // 连接到币安WebSocket 服务器
        connect("stream.binance.com","9443" ,path);
}

void BinanceWebSocketClient::subscribe_ticker(
    const std::string& symbol,
    TickCallback callback
) {
    ticker_callback_ = callback;
    // 转换为小写
    std::string lower_symbol = symbol;
    std::transform(lower_symbol.begin(), lower_symbol.end(),
                        lower_symbol.begin(), ::tolower);
    // 币安 TickerWebSocket 路径 /ws/btcusdt@ticker
    std::string path = "/ws/" + lower_symbol + "@ticker";
    std::cout << "[Binance] 订阅Ticker: " << symbol << std::endl;
    // 连接到币安WebSocket 服务器
    connect("stream.binance.com","9443" ,path);
}

// 消息解析实现
void BinanceWebSocketClient::parse_message(const std::string& message){
    try{
        auto j = nlohmann::json::parse(message);

        // 检查消息类型   为什么消息类型可以这么来检查， 这里检查消息类型是根据什么来确定的
        if(j.contains("e")){
            std::string event_type = j["e"].get<std::string>();
            if (event_type == "kline") {
                parse_kline_message(j);
            } else if (event_type == "24hrTicker") {
                parse_ticker_message(j);
            } else {
                std::cout << "[Binance] 未知事件类型: " << event_type << std::endl;
            }
        } else {
            // 可能是订阅确认或其他消息
            std::cout << "[Binance] 收到消息: " << message.substr(0, 100) << "..." << std::endl;
        }
    } catch (const std::exception& e){
        std::cerr << "[Binance] JSON解析失败: " << e.what() << std::endl;
    }
}

void BinanceWebSocketClient::parse_kline_message(const nlohmann::json& j) {
    // 调试：打印原始消息
    std::cout << "[Binance] 原始消息: " << j.dump().substr(0, 200) << std::endl;
    
/*
    * 币安K线消息格式:
    * {
    *   "e": "kline",
    *   "E": 1638747660000,     // 事件时间
    *   "s": "BTCUSDT",         // 交易对
    *   "k": {
    *     "t": 1638747600000,   // K线开始时间
    *     "T": 1638747659999,   // K线结束时间
    *     "s": "BTCUSDT",       // 交易对
    *     "i": "1m",            // 时间间隔
    *     "o": "50000.00",      // 开盘价
    *     "c": "50100.00",      // 收盘价
    *     "h": "50200.00",      // 最高价
    *     "l": "49900.00",      // 最低价
    *     "v": "100.5",         // 成交量
    *     "x": false            // 是否已完成
    *   }
    * }
    */
    if(!j.contains("k")){
        std::cerr << "[Binance] K线消息缺少k字段" << std::endl;
        return;
    }

    const auto& k = j["k"];
    OHLCV ohlcv;
    ohlcv.timestamp = k["t"].get<uint64_t>();
    ohlcv.open = std::stod(k["o"].get<std::string>());
    ohlcv.high = std::stod(k["h"].get<std::string>());
    ohlcv.low = std::stod(k["l"].get<std::string>());
    ohlcv.close = std::stod(k["c"].get<std::string>());
    ohlcv.volume = std::stod(k["v"].get<std::string>());
    ohlcv.quote_volume = std::stod(k["q"].get<std::string>());

    // 检查是否是已经完成的K线        这里已完成的K线是什么意思
    bool is_closed = k["x"].get<bool>();

    // 打印实时数据
    // 打印实时数据
    std::cout << "[Binance] K线数据: "
              << "O=" << ohlcv.open << " "
              << "H=" << ohlcv.high << " "
              << "L=" << ohlcv.low << " "
              << "C=" << ohlcv.close << " "
              << "V=" << ohlcv.volume
              << (is_closed ? " [已完成]" : " [进行中]")
              << std::endl;

    // 调用用户回调， 这里回调的目的是什么
    if(kline_callback_){
        kline_callback_(ohlcv);
    }
}


void BinanceWebSocketClient::parse_ticker_message(const nlohmann::json& j) {
    /*
     * 币安24小时Ticker消息格式:
     * {
     *   "e": "24hrTicker",
     *   "s": "BTCUSDT",         // 交易对
     *   "c": "50000.00",        // 最新价格
     *   "o": "49000.00",        // 24小时开盘价
     *   "h": "51000.00",        // 24小时最高价
     *   "l": "48000.00",        // 24小时最低价
     *   "v": "10000.5",         // 24小时成交量
     *   "p": "1000.00",         // 价格变化
     *   "P": "2.04"             // 价格变化百分比
     * }
     */
    
    Ticker ticker;
    ticker.symbol = j["s"].get<std::string>();
    ticker.last = std::stod(j["c"].get<std::string>());
    ticker.high_24h = std::stod(j["h"].get<std::string>());
    ticker.low_24h = std::stod(j["l"].get<std::string>());
    ticker.volume_24h = std::stod(j["v"].get<std::string>());
    ticker.change_24h = std::stod(j["p"].get<std::string>());
    ticker.timestamp = j["E"].get<uint64_t>();
    
    std::cout << "[Binance] Ticker: "
              << ticker.symbol << " "
              << "价格=" << ticker.last << " "
              << "涨跌=" << ticker.change_24h << "%"
              << std::endl;
    
    // 调用用户回调
    if (ticker_callback_) {
        ticker_callback_(ticker);
    }
}

// ============= 构建订阅消息 ==================
std::string BinanceWebSocketClient::build_subscribe_message(
    const std::string& channel,
    const std::string& symbol
){
    /*
     * 币安组合流订阅消息格式:
     * {
     *   "method": "SUBSCRIBE",
     *   "params": ["btcusdt@kline_1m"],
     *   "id": 1
     * }
     * 
     * 注意：单流模式不需要发送订阅消息，直接连接即可
     */
    std::string lower_symbol = symbol;
    // 这里是把字符串全部转换为小写的
    std::transform(lower_symbol.begin(), lower_symbol.end(),
                        lower_symbol.begin(), ::tolower);
    nlohmann::json subscribe_msg = {
        {"method", "SUBSCRIBE"},
        {"params", {lower_symbol + "@" + channel}},
        {"id", 1}};
    return subscribe_msg.dump();
}




}
}



