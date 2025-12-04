#pragma once

#include<vector>
#include<cstdint>
#include "common/types.h"

namespace quant_crypto {
namespace analysis {

struct PerformanceMetrics {
    // ===== 收益指标 =====
    double annualized_return;     // 年化收益率
    double cumulative_return;     // 累计收益率
    std::vector<double> equity_curve;     //  权益曲线数据，每个时间点对应的权益值

    // 风险指标 ------   这些风险指标分别是什么意思
    double max_drawdown;       // 最大回撤
    double sharpe_ratio;       // 夏普比率（无风险利率=0）
    double sortino_ratio;      // 索提诺比率
    double calmar_ratio;       // 卡尔玛比率
    double volatility;         // 波动率 （年化， %）
    double downside_deviation;  // 下行波动率

    // 交易指标
    double profit_loss_ratio;     // 平均盈亏比
    int max_consecutive_wins;     // 最大连续盈利次数
    int max_consecutive_losses;     // 最大连续亏损次数
    double avg_holding_period;    // 平均持仓时间
    double trade_frequency_per_year;    // 交易频率

    // 回撤数据 ===============
    std::vector<double> drawdown_curve;      // 回撤曲线数据（每个Bar的回撤）


    // 构造函数：初始化所有指标为0
    PerformanceMetrics() : annualized_return(0.0), cumulative_return(0.0), max_drawdown(0.0), sharpe_ratio(0.0), sortino_ratio(0.0), calmar_ratio(0.0), volatility(0.0), downside_deviation(0.0), profit_loss_ratio(0.0), max_consecutive_wins(0), max_consecutive_losses(0), avg_holding_period(0.0), trade_frequency_per_year(0.0) {}

};




}
}

