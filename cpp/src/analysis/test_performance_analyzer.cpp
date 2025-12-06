/**
 * @file test_performance_analyzer.cpp
 * @brief 性能分析器测试程序
 */

#include "analysis/performance_analyzer.h"
#include "backtest/backtest_engine.h"
#include "strategy/ma_cross_strategy.h"
#include "collectors/binance_collector.h"
#include "config/config_manager.h"
#include <iostream>
#include <iomanip>

using namespace quant_crypto;

/**
 * @brief 打印性能指标
 */
void print_performance_metrics(const analysis::PerformanceMetrics& metrics) {
    std::cout << "\n==============================================\n";
    std::cout << "           性能分析报告\n";
    std::cout << "==============================================\n";
    
    // ========== 收益指标 ==========
    std::cout << "\n【收益指标】\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  年化收益率:         " << (metrics.annualized_return * 100) << "%\n";
    std::cout << "  累计收益率:         " << (metrics.cumulative_return * 100) << "%\n";
    std::cout << "  权益曲线数据点数:    " << metrics.equity_curve.size() << "\n";
    
    // ========== 风险指标 ==========
    std::cout << "\n【风险指标】\n";
    std::cout << "  最大回撤:           " << (metrics.max_drawdown * 100) << "%\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  夏普比率:           " << metrics.sharpe_ratio << "\n";
    std::cout << "  索提诺比率:         " << metrics.sortino_ratio << "\n";
    std::cout << "  卡玛比率:           " << metrics.calmar_ratio << "\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  波动率:             " << (metrics.volatility * 100) << "%\n";
    std::cout << "  下行波动率:         " << (metrics.downside_deviation * 100) << "%\n";
    
    // ========== 交易指标 ==========
    std::cout << "\n【交易指标】\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  盈亏比:             " << metrics.profit_loss_ratio << "\n";
    std::cout << "  最大连续盈利:       " << metrics.max_consecutive_wins << " 次\n";
    std::cout << "  最大连续亏损:       " << metrics.max_consecutive_losses << " 次\n";
    std::cout << "  平均持仓时间:       " << metrics.avg_holding_period << " 天\n";
    std::cout << "  年化交易频率:       " << metrics.trade_frequency_per_year << " 次/年\n";
    
    // ========== 回撤曲线 ==========
    std::cout << "\n【回撤曲线】\n";
    std::cout << "  回撤曲线数据点数:    " << metrics.drawdown_curve.size() << "\n";
    
    // 打印最大回撤发生的位置
    if (!metrics.drawdown_curve.empty()) {
        auto max_dd_it = std::max_element(metrics.drawdown_curve.begin(), 
                                         metrics.drawdown_curve.end());
        size_t max_dd_index = std::distance(metrics.drawdown_curve.begin(), max_dd_it);
        std::cout << "  最大回撤位置:       第 " << max_dd_index << " 个Bar\n";
    }
    
    std::cout << "\n==============================================\n";
}

/**
 * @brief 主函数：测试性能分析器
 */
int main() {
    std::cout << "========== 性能分析器测试程序 ==========\n\n";
    
    // ========== 第1步：创建配置 ==========
    std::cout << "[1] 初始化配置...\n";
    config::BinanceConfig binance_config;
    binance_config.base_url = "https://api.binance.com";
    binance_config.proxy_enabled = false;
    binance_config.timeout_ms = 10000;
    binance_config.data_dir = "data";
    std::cout << "  Base URL: " << binance_config.base_url << "\n";
    
    // ========== 第2步：获取历史数据 ==========
    std::cout << "\n[2] 获取历史K线数据...\n";
    collectors::BinanceCollector collector(binance_config);
    
    auto klines_result = collector.get_klines("BTCUSDT", "1h", 500);
    if (!klines_result.success || klines_result.data.empty()) {
        std::cerr << "获取K线失败: " << klines_result.error_message << "\n";
        return 1;
    }
    
    std::cout << "  获取到 " << klines_result.data.size() << " 条K线数据\n";
    std::cout << "  时间范围: " << klines_result.data.front().timestamp 
              << " -> " << klines_result.data.back().timestamp << "\n";
    
    // ========== 第3步：运行回测 ==========
    std::cout << "\n[3] 运行回测引擎...\n";
    
    // 创建策略
    strategy::MACrossConfig strategy_config;
    strategy_config.fast_period = 10;
    strategy_config.slow_period = 30;
    strategy_config.position_size = 1.0;  // 全仓
    
    strategy::MACrossStrategy strategy(strategy_config);
    
    // 创建回测引擎
    backtest::BacktestConfig backtest_config;
    backtest_config.initial_capital = 10000.0;
    backtest_config.commission_rate = 0.001;  // 0.1%
    backtest_config.slippage_rate = 0.0005;   // 0.05%
    
    backtest::BacktestEngine engine(backtest_config);
    engine.set_strategy(&strategy);
    engine.set_data(klines_result.data);
    
    engine.run();
    
    auto backtest_result = engine.get_result();
    
    std::cout << "  初始资金: " << backtest_config.initial_capital << " USDT\n";
    std::cout << "  最终权益: " << backtest_result.final_equity << " USDT\n";
    std::cout << "  总收益率: " << backtest_result.total_return << "%\n";
    std::cout << "  交易次数: " << backtest_result.trades.size() << "\n";
    std::cout << "  盈利次数: " << backtest_result.winning_trades << "\n";
    std::cout << "  亏损次数: " << backtest_result.losing_trades << "\n";
    std::cout << "  权益曲线数据点: " << backtest_result.equity_curve.size() << "\n";
    
    // ========== 第4步：性能分析 ==========
    std::cout << "\n[4] 进行性能分析...\n";
    
    analysis::PerformanceAnalyzer analyzer;
    
    auto metrics = analyzer.analyze(
        backtest_result.equity_curve,      // 权益曲线
        backtest_result.timestamps,        // 时间戳
        backtest_result.trades,            // 交易记录
        backtest_config.initial_capital    // 初始资金
    );
    
    // ========== 第5步：打印性能报告 ==========
    print_performance_metrics(metrics);
    
    // ========== 第6步：详细的权益曲线分析 ==========
    std::cout << "\n【权益曲线详情（前10个点和后10个点）】\n";
    std::cout << std::fixed << std::setprecision(2);
    
    size_t show_count = std::min<size_t>(10, metrics.equity_curve.size());
    
    std::cout << "  前" << show_count << "个点:\n";
    for (size_t i = 0; i < show_count; i++) {
        std::cout << "    Bar " << i << ": " << metrics.equity_curve[i] 
                  << " USDT (回撤: " << (metrics.drawdown_curve[i] * 100) << "%)\n";
    }
    
    std::cout << "  后" << show_count << "个点:\n";
    for (size_t i = metrics.equity_curve.size() - show_count; 
         i < metrics.equity_curve.size(); i++) {
        std::cout << "    Bar " << i << ": " << metrics.equity_curve[i] 
                  << " USDT (回撤: " << (metrics.drawdown_curve[i] * 100) << "%)\n";
    }
    
    std::cout << "\n========== 测试完成 ==========\n";
    
    return 0;
}

