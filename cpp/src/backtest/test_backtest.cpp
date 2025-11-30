#include "backtest/backtest_engine.h"
#include "strategy/ma_cross_strategy.h"
#include "collectors/binance_collector.h"
#include "config/config_manager.h"
#include <iostream>
#include <iomanip>

using namespace quant_crypto;

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "回测引擎测试" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // 1. 加载配置
    if (!config::ConfigManager::load("../config/binance.json")) {
        std::cerr << "❌ 配置加载失败" << std::endl;
        return 1;
    }
    
    // 2. 获取历史数据
    auto config = config::ConfigManager::get_binance_config();
    collectors::BinanceCollector collector(config);
    auto result = collector.get_klines("BTCUSDT", "1h", 100);
    
    if (!result.success) {
        std::cerr << "❌ 数据获取失败：" << result.error_message << std::endl;
        return 1;
    }
    std::cout << "✅ 获取 " << result.data.size() << " 条K线数据\n" << std::endl;
    
    // 3. 创建策略
    strategy::MACrossConfig strategy_config;
    strategy_config.fast_period = 5;
    strategy_config.slow_period = 20;
    strategy_config.position_size = 0.5;
    
    strategy::MACrossStrategy strategy(strategy_config);
    
    // 4. 创建回测引擎
    backtest::BacktestConfig backtest_config;
    backtest_config.initial_capital = 10000.0;
    backtest_config.commission_rate = 0.001;  // 0.1%手续费
    backtest_config.slippage_rate = 0.0005;   // 0.05%滑点
    
    backtest::BacktestEngine engine(backtest_config);
    
    // 5. 运行回测
    engine.set_strategy(&strategy);
    engine.set_data(result.data);
    
    std::cout << "[回测] 开始运行..." << std::endl;
    engine.run();
    std::cout << "[回测] 运行完成\n" << std::endl;
    
    // 6. 获取并打印结果
    auto backtest_result = engine.get_result();
    
    std::cout << "========================================" << std::endl;
    std::cout << "回测结果" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "初始资金: $" << backtest_result.initial_capital << std::endl;
    std::cout << "最终资金: $" << backtest_result.final_capital << std::endl;
    std::cout << "最终权益: $" << backtest_result.final_equity << std::endl;
    std::cout << "总收益率: " << backtest_result.total_return << "%" << std::endl;
    std::cout << "总交易次数: " << backtest_result.total_trades << std::endl;
    std::cout << "盈利交易: " << backtest_result.winning_trades << std::endl;
    std::cout << "亏损交易: " << backtest_result.losing_trades << std::endl;
    
    if (backtest_result.total_trades > 0) {
        double win_rate = (double)backtest_result.winning_trades / backtest_result.total_trades * 100;
        std::cout << "胜率: " << win_rate << "%" << std::endl;
    }
    
    // 7. 打印交易记录
    if (!backtest_result.trades.empty()) {
        std::cout << "\n交易记录:" << std::endl;
        std::cout << std::string(60, '-') << std::endl;
        for (const auto& trade : backtest_result.trades) {
            std::cout << (trade.signal == strategy::Signal::BUY ? "📈 买入" : "📉 卖出");
            std::cout << " | 价格: $" << trade.price;
            if (trade.signal == strategy::Signal::SELL) {
                std::cout << " | 盈亏: $" << trade.pnl;
            }
            std::cout << std::endl;
        }
    }
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "测试完成" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}