#include "collectors/binance_collector.h"
#include <iostream>
#include "storage/kline_storage.h"
#include "config/config_manager.h"
#include "common/types.h"

using namespace quant_crypto::collectors;
using namespace quant_crypto;

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "测试 BinanceCollector 类" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    std::cout << "正在加载配置文件..." << std::endl;
    // 从 build/ 目录运行，需要向上一级到项目根目录
    if (!quant_crypto::config::ConfigManager::load("../config/binance.json")) {
        std::cerr << "❌ 配置文件加载失败！程序退出。" << std::endl;
        return 1;
    }
    // 2. 获取币安配置
    auto config = quant_crypto::config::ConfigManager::get_binance_config();

    // 3. 创建 BinanceCollector 实例
    BinanceCollector collector(config);

    // // 1. 配置 BinanceCollector
    // BinanceConfig config;
    // config.base_url = "https://api.binance.com";
    // config.proxy_host = "127.0.0.1";  // 你的代理地址
    // config.proxy_port = 7897;         // 你的代理端口
    // config.timeout_ms = 30000;         // 30秒超时
    
    // // 2. 创建 BinanceCollector 实例
    // BinanceCollector collector(config);
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "开始获取 BTC/USDT K线数据..." << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // 3. 获取K线数据
    auto result = collector.get_klines("BTCUSDT", "1h", 5);
    
    // 4. 检查结果
    if (result.success) {
        std::cout << "\n✅ 成功获取 " << result.data.size() << " 条K线数据！\n" << std::endl;
        
        // 5. 打印每条OHLCV数据
        std::cout << "OHLCV数据详情：" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        
        int i = 1;
        for (const auto& ohlcv : result.data) {
            std::cout << "K线 " << i++ << ":" << std::endl;
            std::cout << "  时间戳: " << ohlcv.timestamp << std::endl;
            std::cout << "  交易对: " << ohlcv.symbol << std::endl;
            std::cout << "  交易所: " << ohlcv.exchange << std::endl;
            std::cout << "  周期: " << timeframe_to_string(ohlcv.timeframe) << std::endl;
            std::cout << "  开盘价: " << ohlcv.open << std::endl;
            std::cout << "  最高价: " << ohlcv.high << std::endl;
            std::cout << "  最低价: " << ohlcv.low << std::endl;
            std::cout << "  收盘价: " << ohlcv.close << std::endl;
            std::cout << "  成交量: " << ohlcv.volume << std::endl;
            std::cout << "  成交额: " << ohlcv.quote_volume << std::endl;
            std::cout << "  成交笔数: " << ohlcv.trades_count << std::endl;
            std::cout << "  质量: " << static_cast<int>(ohlcv.quality) << std::endl;
            std::cout << std::endl;
        }
        
        // 创建存储对象（使用配置文件中的路径）
        quant_crypto::storage::KlineStorage storage(config.data_dir);
        
        // 保存OHLCV数据到CSV
        bool save_success = storage.save_ohlcv("BTCUSDT", "1h", result.data);
        
        if (save_success) {
            std::cout << "\n✅ 数据已成功保存到CSV文件！" << std::endl;
            std::cout << "文件位置: data/BTCUSDT_1h_YYYYMMDD.csv" << std::endl;
            std::cout << "可以用文本编辑器或Excel打开查看" << std::endl;
        } else {
            std::cerr << "\n❌ 数据保存失败！" << std::endl;
        }

        std::cout << "========================================" << std::endl;
        std::cout << "✅ 测试成功！BinanceCollector 工作正常！" << std::endl;
        std::cout << "========================================" << std::endl;
        
    } else {
        std::cerr << "\n❌ 获取OHLCV数据失败！" << std::endl;
        std::cerr << "错误信息: " << result.error_message << std::endl;
    }


    // ============================================
    // 测试2：获取24小时价格统计
    // ============================================
    
    // std::cout << "\n\n========================================" << std::endl;
    // std::cout << "测试 get_ticker() - 获取24小时价格统计" << std::endl;
    // std::cout << "========================================\n" << std::endl;
    
    // // 获取Ticker数据
    // auto ticker_result = collector.get_ticker("BTCUSDT");
    
    // if (ticker_result.success) {
    //     std::cout << "\n✅ 成功获取Ticker数据！\n" << std::endl;
        
    //     const auto& ticker = ticker_result.data;
        
    //     std::cout << "Ticker 数据详情：" << std::endl;
    //     std::cout << "----------------------------------------" << std::endl;
    //     std::cout << "交易对: " << ticker.symbol << std::endl;
    //     std::cout << "最新价格: " << ticker.last_price << " USDT" << std::endl;
    //     std::cout << "24h最高价: " << ticker.high_price << " USDT" << std::endl;
    //     std::cout << "24h最低价: " << ticker.low_price << " USDT" << std::endl;
    //     std::cout << "24h成交量: " << ticker.volume << " BTC" << std::endl;
    //     std::cout << "24h涨跌幅: " << ticker.price_change_percent << "%" << std::endl;
    //     std::cout << "----------------------------------------" << std::endl;
        
    //     std::cout << "\n使用 to_string() 方法：" << std::endl;
    //     std::cout << ticker.to_string() << std::endl;
        
    //     std::cout << "\n========================================" << std::endl;
    //     std::cout << "✅ Ticker测试成功！" << std::endl;
    //     std::cout << "========================================" << std::endl;
        
    // } else {
    //     std::cerr << "\n❌ 获取Ticker数据失败！" << std::endl;
    //     std::cerr << "错误信息: " << ticker_result.error_message << std::endl;
    // }
    // ============================================
    // 测试3：获取订单簿深度
    // ============================================
    
    // std::cout << "\n\n========================================" << std::endl;
    // std::cout << "测试 get_orderbook() - 获取订单簿深度" << std::endl;
    // std::cout << "========================================\n" << std::endl;
    
    // auto orderbook_result = collector.get_orderbook("BTCUSDT", 5);  // 只获取5档
    
    // if (orderbook_result.success) {
    //     std::cout << "\n✅ 成功获取OrderBook数据！\n" << std::endl;
        
    //     const auto& ob = orderbook_result.data;
        
    //     std::cout << "订单簿概况：" << std::endl;
    //     std::cout << ob.to_string() << std::endl;
    //     std::cout << "\n买盘（Bids）- 前5档：" << std::endl;
    //     std::cout << "----------------------------------------" << std::endl;
    //     std::cout << "价格\t\t\t数量" << std::endl;
    //     for (size_t i = 0; i < std::min(size_t(5), ob.bids.size()); i++) {
    //         std::cout << ob.bids[i].price << "\t\t" 
    //                   << ob.bids[i].quantity << std::endl;
    //     }
        
    //     std::cout << "\n卖盘（Asks）- 前5档：" << std::endl;
    //     std::cout << "----------------------------------------" << std::endl;
    //     std::cout << "价格\t\t\t数量" << std::endl;
    //     for (size_t i = 0; i < std::min(size_t(5), ob.asks.size()); i++) {
    //         std::cout << ob.asks[i].price << "\t\t" 
    //                   << ob.asks[i].quantity << std::endl;
    //     }
        
    //     std::cout << "\n========================================" << std::endl;
    //     std::cout << "✅ OrderBook测试成功！" << std::endl;
    //     std::cout << "========================================" << std::endl;
        
    // } else {
    //     std::cerr << "\n❌ 获取OrderBook数据失败！" << std::endl;
    //     std::cerr << "错误信息: " << orderbook_result.error_message << std::endl;
    // }
    
    return 0;    

}


