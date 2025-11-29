#include "websocket/binance_websocket_client.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

using namespace quant_crypto;
using namespace quant_crypto::websocket;

// 全局计数器
std::atomic<int> message_count(0);

// K线数据回调函数
void on_kline_received(const OHLCV& ohlcv) {
    message_count++;
    
    // 打印接收到的K线数据
    std::cout << "\n=== 收到K线数据 #" << message_count << " ===" << std::endl;
    std::cout << "交易对: " << ohlcv.symbol << std::endl;
    std::cout << "交易所: " << ohlcv.exchange << std::endl;
    std::cout << "时间戳: " << ohlcv.timestamp << std::endl;
    std::cout << "周期: " << timeframe_to_string(ohlcv.timeframe) << std::endl;
    std::cout << "开盘价: " << ohlcv.open << std::endl;
    std::cout << "最高价: " << ohlcv.high << std::endl;
    std::cout << "最低价: " << ohlcv.low << std::endl;
    std::cout << "收盘价: " << ohlcv.close << std::endl;
    std::cout << "成交量: " << ohlcv.volume << std::endl;
    std::cout << "成交额: " << ohlcv.quote_volume << std::endl;
    std::cout << "成交笔数: " << ohlcv.trades_count << std::endl;
    std::cout << "============================\n" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "WebSocket 实时数据测试" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // 1. 创建WebSocket客户端
    std::cout << "[步骤1] 创建WebSocket客户端..." << std::endl;
    BinanceWebSocketClient client;
    std::cout << "✅ 客户端创建成功\n" << std::endl;
    
    // 2. 订阅BTC 1分钟K线数据
    std::cout << "[步骤2] 订阅 BTCUSDT 1分钟K线数据..." << std::endl;
    bool success = client.subscribe_kline("btcusdt", "1m", on_kline_received);
    
    if (!success) {
        std::cerr << "❌ 订阅失败！" << std::endl;
        return 1;
    }
    
    std::cout << "✅ 订阅成功！等待数据...\n" << std::endl;
    
    // 3. 等待连接建立
    std::cout << "[步骤3] 等待连接建立..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    if (client.is_connected()) {
        std::cout << "✅ WebSocket已连接！\n" << std::endl;
    } else {
        std::cerr << "❌ 连接失败！" << std::endl;
        return 1;
    }
    
    // 4. 接收数据（运行30秒）
    std::cout << "========================================" << std::endl;
    std::cout << "🚀 开始接收实时数据..." << std::endl;
    std::cout << "（程序将运行30秒，按Ctrl+C可提前退出）" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // 每5秒显示一次统计
    for (int i = 0; i < 6; i++) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::cout << "[统计] 已接收 " << message_count << " 条消息" << std::endl;
    }
    
    // 5. 停止WebSocket
    std::cout << "\n========================================" << std::endl;
    std::cout << "[步骤4] 停止WebSocket连接..." << std::endl;
    client.stop();
    
    // 6. 总结
    std::cout << "\n========================================" << std::endl;
    std::cout << "✅ 测试完成！" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "总计接收消息: " << message_count << " 条" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}