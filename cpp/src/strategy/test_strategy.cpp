#include "strategy/ma_cross_strategy.h"
#include "collectors/binance_collector.h"
#include "config/config_manager.h"
#include <iostream>

using namespace quant_crypto;
using namespace quant_crypto::strategy;
using namespace quant_crypto::collectors;
using namespace quant_crypto::config;



int main() {
    // 1. 加载配置
    ConfigManager::load("../config/binance.json");
    
    // 2. 获取历史数据
    auto config = ConfigManager::get_binance_config();
    BinanceCollector collector(config);
    auto result = collector.get_klines("BTCUSDT", "1h", 100);
    
    if (!result.success) {
        std::cerr << "❌ 获取数据失败：" << result.error_message << std::endl;
        return 1;
    }
    std::cout << "✅ 获取 " << result.data.size() << " 条K线数据\n" << std::endl;
    // 3. 创建策略
    MACrossConfig strategy_config;
    strategy_config.fast_period = 5;
    strategy_config.slow_period = 20;
    strategy_config.position_size = 0.5;  // 50%仓位
    
    MACrossStrategy strategy(strategy_config);
    strategy.on_init(10000.0);  // 初始资金$10,000
    
    // 4. 回测循环
    for (const auto& bar : result.data) {
        // 喂入K线
        strategy.on_bar(bar);
        
        // 生成信号
        Signal signal = strategy.generate_signal();
        
        // 执行交易
        if (signal == Signal::BUY) {
            // 计算买入数量
            double cost = strategy.get_capital() * 0.5;  // 50%资金
            double quantity = cost / bar.close;
            
            // 开仓
            strategy.open_position(bar.symbol, quantity, bar.close);
            
            // 记录交易
            Trade trade;
            trade.timestamp = bar.timestamp;
            trade.signal = Signal::BUY;
            trade.price = bar.close;
            trade.quantity = quantity;
            strategy.add_trade(trade);
            
            std::cout << "📈 买入：价格=" << bar.close << std::endl;
        }
        else if (signal == Signal::SELL) {
            // 平仓
            double pnl = strategy.close_position(bar.close);
            
            // 记录交易
            Trade trade;
            trade.timestamp = bar.timestamp;
            trade.signal = Signal::SELL;
            trade.price = bar.close;
            trade.pnl = pnl;
            strategy.add_trade(trade);
            
            std::cout << "📉 卖出：价格=" << bar.close 
                      << "，盈亏=" << pnl << std::endl;
        }
    }
    
    // 5. 打印统计
    std::cout << "总资金：" << strategy.get_capital() << std::endl;
    std::cout << "总权益：" << strategy.get_total_equity() << std::endl;
    std::cout << "收益率：" << strategy.get_total_return() << "%" << std::endl;
}