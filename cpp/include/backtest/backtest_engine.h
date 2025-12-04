#pragma once

#include "common/types.h"
#include "strategy/strategy_base.h"
#include <vector>

namespace quant_crypto {
namespace backtest {

// 回测参数配置
struct BacktestConfig {
    double initial_capital;   // 初始资金
    double commission_rate;   // 手续费率
    double slippage_rate;   // 滑点率

    BacktestConfig():
        initial_capital(10000.0),
        commission_rate(0.001),
        slippage_rate(0.001)
    {}
};

// 保存回测结果
struct BacktestResult{
    double initial_capital;   // 初始资金
    double final_capital;  // 最终资金
    double final_equity;  // 最终受益
    double total_return;  // 总收益率(资金+持仓)
    int total_trades;   // 总交易次数
    int winning_trades;   // 亏损交易次数
    int losing_trades;   // 亏损交易次数
    std::vector<strategy::Trade> trades;  //交易记录

    // ========  权益曲线数据
    std::vector<double> equity_curve;     // 权益曲线（每个Bar的总权益）
    std::vector<Timestamp> timestamps;     // 时间戳(每个Bar的时间戳)

    BacktestResult()
        : initial_capital(0), final_capital(0),final_equity(0),
        total_return(0), total_trades(0), winning_trades(0), losing_trades(0){}
};

// 3. BacktestEngine类
class BacktestEngine {
public:
    // 这里加上explicit目的是什么？防止隐式转换
    explicit BacktestEngine(const BacktestConfig& config);
    ~BacktestEngine() = default;

    // 公有接口
    void set_strategy(strategy::StrategyBase* strategy);
    void set_data(const std::vector<OHLCV>& data);
    void run();
    BacktestResult get_result() const;

private:
    BacktestConfig config_;
    strategy::StrategyBase* strategy_;
    std::vector<OHLCV> data_;
    BacktestResult result_;

    // 私有方法  这三个私有方法具体是干什么的
    // 处理交易信号， 执行买入/卖出 操作
    void process_signal(strategy::Signal signal,const OHLCV& bar);
    // 计算手续费
    double calculate_commission(double amount);
    // 计算滑点
    double calculate_slippage(double price);

};
}
}


