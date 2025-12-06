#pragma once

#include "analysis/performance_metrics.h"
#include "strategy/strategy_base.h"
#include "common/types.h"
#include <vector>


namespace quant_crypto {
namespace analysis{

    /** 性能分析器，（后处理计算方式） 
    * 计算收益指标
    * 计算风险指标
    * 计算交易指标
    */
class PerformanceAnalyzer{
public:

    /**
     * @brief 分析回测结果
     * @param equity_curve 权益曲线（每个Bar的总权益）
     * @param timestamps 时间戳（每个Bar的时间戳）
     * @param trades 交易记录
     * @param initial_capital 初始资金
     * @return 性能指标
     */
    PerformanceMetrics analyze(
        const std::vector<double>& equity_curve,
        const std::vector<Timestamp>& timestamps,
        const std::vector<strategy::Trade>& trades,
        double initial_captial
    );

private:
    // ============ 收益指标计算 =============
    // 年化收益率
    double calculate_annualized_return(
        double initial_captial,
        double final_captial,
        Timestamp start_time,
        Timestamp end_time
    );

    // 累计收益率
    double calculate_cumlative_return(
        double initial_captial,
        double final_captial
    );

    // ============ 风险指标计算  ============
    // 最大回撤
    double calculate_max_drawdown(const std::vector<double>& equity_curve);
    // 夏普比率
    double calculate_sharpe_ratio(const std::vector<double>& returns);
    // 索提诺比率
    double calculate_sortino_ratio(const std::vector<double>& returns);
    // 卡尔玛比率
    double calculate_calmar_ratio(
        double annualized_return,
        double max_drawdown
    );
    // 波动率
    double calculate_volatility(const std::vector<double>& returns);
    // 下行波动率
    double calculate_downside_deviation(const std::vector<double>& returns);

    // =============  交易指标计算 ==========
    // 平均盈亏比
    double calculate_profit_loss_ratio(const std::vector<strategy::Trade>& trades);
    // 最大连续盈利次数和最大连续亏损次数
    std::pair<int, int> calculate_max_consecutive(
        const std::vector<strategy::Trade>& trades
    );
    // 平均持仓时间
    double calculate_avg_holding_period(
        const std::vector<strategy::Trade>& trades
    );
    // 交易频率
    double calculate_trade_frequency(
        int total_trades,
        Timestamp start_time,
        Timestamp end_time
    );

    // ========= 辅助方法 ========
    // 计算收益率
    std::vector<double> calculate_returns(const std::vector<double>& equity_curve);
    // 计算回撤曲线
    std::vector<double> calculate_drawdown_curve(const std::vector<double>& equity_curve);

};






}
}






