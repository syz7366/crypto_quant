#include "analysis/performance_analyzer.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <vector>

namespace quant_crypto {
namespace analysis{


PerformanceMetrics PerformanceAnalyzer::analyze(
    const std::vector<double>& equity_curve,
    const std::vector<Timestamp>& timestamps,
    const std::vector<strategy::Trade>& trades,
    double initial_captial
){
    PerformanceMetrics metrics;

    // 参数验证
    if (equity_curve.empty() || timestamps.empty() || 
        equity_curve.size() != timestamps.size()) {
        return metrics;  // 返回空指标
    }


    // 保存权益曲线
    metrics.equity_curve = equity_curve;
    
    // 计算收益率序列
    std::vector<double> returns = calculate_returns(equity_curve);
    
    // ========== 1. 计算收益指标 ==========
    double final_capital = equity_curve.back();
    metrics.cumulative_return = calculate_cumlative_return(initial_captial, final_capital);
    metrics.annualized_return = calculate_annualized_return(
        initial_captial, 
        final_capital, 
        timestamps.front(), 
        timestamps.back()
    );
    
    // ========== 2. 计算风险指标 ==========
    metrics.max_drawdown = calculate_max_drawdown(equity_curve);
    metrics.volatility = calculate_volatility(returns);
    metrics.downside_deviation = calculate_downside_deviation(returns);
    metrics.sharpe_ratio = calculate_sharpe_ratio(returns);
    metrics.sortino_ratio = calculate_sortino_ratio(returns);
    metrics.calmar_ratio = calculate_calmar_ratio(
        metrics.annualized_return, 
        metrics.max_drawdown
    );
    
    // 计算并保存回撤曲线
    metrics.drawdown_curve = calculate_drawdown_curve(equity_curve);
    
    // ========== 3. 计算交易指标 ==========
    metrics.profit_loss_ratio = calculate_profit_loss_ratio(trades);
    
    auto [max_wins, max_losses] = calculate_max_consecutive(trades);
    metrics.max_consecutive_wins = max_wins;
    metrics.max_consecutive_losses = max_losses;
    
    metrics.avg_holding_period = calculate_avg_holding_period(trades);
    
    // 只统计卖出交易（平仓）的次数
    int sell_count = 0;
    for (const auto& trade : trades) {
        if (trade.signal == strategy::Signal::SELL) {
            sell_count++;
        }
    }
    metrics.trade_frequency_per_year = calculate_trade_frequency(
        sell_count,
        timestamps.front(),
        timestamps.back()
    );
    
    return metrics;
}


// 辅助方法： 计算收益率序列
std::vector<double> PerformanceAnalyzer::calculate_returns(
    const std::vector<double>& equity_curve) {
// ========= 辅助方法：计算收益率序列 =========
    std::vector<double> returns;
    if(equity_curve.size()<2) return returns;
    
    returns.reserve(equity_curve.size()-1);

    for(size_t i=1;i<equity_curve.size();i++){
        if(equity_curve[i-1]>0){
            double ret = (equity_curve[i]-equity_curve[i-1])/equity_curve[i-1];
            returns.push_back(ret);
        }
    }
    return returns;
}

// =========== 辅助方法：计算回撤曲线  ===========
std::vector<double> PerformanceAnalyzer::calculate_drawdown_curve(
    const std::vector<double>& equity_curve
){
    std::vector<double> drawdown_curve;
    if(equity_curve.empty()) return drawdown_curve;

    drawdown_curve.reserve(equity_curve.size());
    double peak = equity_curve[0];

    for(double equity : equity_curve){
        if(equity>peak) peak = equity;
        double drawdown = (peak-equity)/peak;
        drawdown_curve.push_back(drawdown);
    }
    return drawdown_curve;
}

//============= 收益指标： 年化收益率 =============
double PerformanceAnalyzer::calculate_annualized_return(
    double initial_captial,
    double final_captial,
    Timestamp start_time,
    Timestamp end_time
){
    if (initial_captial <= 0 || final_captial <= 0) {
        return 0.0;
    }
    
    // 计算总收益率
    double total_return = (final_captial - initial_captial) / initial_captial;
    
    // 计算时间跨度（秒）
    int64_t time_span_seconds = end_time - start_time;
    if (time_span_seconds <= 0) {
        return 0.0;
    }
    // 转换为年数（假设1年 = 365.25天 = 365.25 * 24 * 3600秒）
    const double SECONDS_PER_YEAR = 365.25 * 24 * 3600;
    double years = static_cast<double>(time_span_seconds) / SECONDS_PER_YEAR;
    
    if (years <= 0) {
        return 0.0;
    }
    // 年化收益率公式：(1 + 总收益率)^(1/年数) - 1
    double annualized = std::pow(1.0 + total_return, 1.0 / years) - 1.0;
    return annualized;
}

// =========== 收益指标： 累计收益率 ==============
double PerformanceAnalyzer::calculate_cumlative_return(
    double initial_captial,
    double final_captial
){
    if (initial_captial <= 0) {
        return 0.0;
    }
    
    // 累计收益率 = (最终资金 - 初始资金) / 初始资金
    return (final_captial - initial_captial) / initial_captial;
}

// ==========  最大回撤 ================
double PerformanceAnalyzer::calculate_max_drawdown(
    const std::vector<double>& equity_curve
) {
    if (equity_curve.empty()) {
        return 0.0;
    }
    
    // 使用回撤曲线，找出最大值
    std::vector<double> drawdown_curve = calculate_drawdown_curve(equity_curve);
    
    if (drawdown_curve.empty()) {
        return 0.0;
    }
    
    // 找出回撤曲线中的最大值
    auto max_it = std::max_element(drawdown_curve.begin(), drawdown_curve.end());
    return *max_it;
}

// ========== 风险指标：波动率（标准差） ==========
double PerformanceAnalyzer::calculate_volatility(
    const std::vector<double>& returns
) {
    if (returns.size() < 2) {
        return 0.0;
    }
    // 计算平均收益率
    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    // 计算方差
    double variance = 0.0;
    for (double ret : returns) {
        double diff = ret - mean;
        variance += diff * diff;
    }
    variance /= returns.size();
    
    // 标准差 = sqrt(方差)
    double std_dev = std::sqrt(variance);
    return std_dev;
}

// ========== 风险指标：下行波动率 ==========
double PerformanceAnalyzer::calculate_downside_deviation(
    const std::vector<double>& returns
) {
    if (returns.empty()) {
        return 0.0;
    }
    
    // 只考虑负收益率（下行）
    std::vector<double> downside_returns;
    for (double ret : returns) {
        if (ret < 0.0) {
            downside_returns.push_back(ret);
        }
    }
    
    if (downside_returns.empty()) {
        return 0.0;
    }
    
    // 计算下行收益率的平均值
    double mean = std::accumulate(downside_returns.begin(), downside_returns.end(), 0.0) 
                  / downside_returns.size();
    
    // 计算下行方差（相对于0，而不是平均值）
    double variance = 0.0;
    for (double ret : downside_returns) {
        variance += ret * ret;  // 相对于0的偏差
    }
    variance /= downside_returns.size();
    
    // 下行标准差
    return std::sqrt(variance);
}

// ========== 风险指标：夏普比率 ==========
double PerformanceAnalyzer::calculate_sharpe_ratio(
    const std::vector<double>& returns
) {
    if (returns.size() < 2) {
        return 0.0;
    }
    
    // 计算平均收益率
    double mean_return = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    
    // 计算波动率
    double volatility = calculate_volatility(returns);
    
    if (volatility == 0.0) {
        return 0.0;
    }
    
    // 夏普比率 = (平均收益率 - 无风险利率) / 波动率
    // 这里假设无风险利率为0（加密货币市场）
    double sharpe = mean_return / volatility;
    
    // 年化夏普比率（假设日收益率）
    // 年化 = 日夏普 * sqrt(252)
    // 这里先返回原始值，后续可以根据Bar频率调整
    return sharpe;
}

// ========== 风险指标：索提诺比率 ==========
double PerformanceAnalyzer::calculate_sortino_ratio(
    const std::vector<double>& returns
) {
    if (returns.empty()) {
        return 0.0;
    }
    
    // 计算平均收益率
    double mean_return = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    
    // 计算下行波动率
    double downside_dev = calculate_downside_deviation(returns);
    
    if (downside_dev == 0.0) {
        return 0.0;
    }
    
    // 索提诺比率 = (平均收益率 - 无风险利率) / 下行波动率
    // 这里假设无风险利率为0
    double sortino = mean_return / downside_dev;
    
    return sortino;
}

// ========== 风险指标：卡玛比率 ==========
double PerformanceAnalyzer::calculate_calmar_ratio(
    double annualized_return,
    double max_drawdown
) {
    if (max_drawdown == 0.0) {
        return 0.0;
    }
    
    // 卡玛比率 = 年化收益率 / 最大回撤
    return annualized_return / max_drawdown;
}

// ========== 交易指标：盈亏比 ==========
double PerformanceAnalyzer::calculate_profit_loss_ratio(
    const std::vector<strategy::Trade>& trades
) {
    if (trades.empty()) {
        return 0.0;
    }
    
    // 浮点数比较的误差范围
    const double EPSILON = 1e-8;
    
    double total_profit = 0.0;
    double total_loss = 0.0;
    int profit_count = 0;
    int loss_count = 0;
    
    // 遍历所有交易，统计盈利和亏损
    for (const auto& trade : trades) {
        // 只统计平仓交易（有盈亏的交易）
        // 使用 EPSILON 判断 pnl 是否为 0
        if (std::abs(trade.pnl) > EPSILON) {
            if (trade.pnl > EPSILON) {  // 盈利
                total_profit += trade.pnl;
                profit_count++;
            } else if (trade.pnl < -EPSILON) {  // 亏损
                total_loss += std::abs(trade.pnl);
                loss_count++;
            }
        }
    }
    
    // 如果没有盈利或亏损，返回0
    if (profit_count == 0 || loss_count == 0) {
        return 0.0;
    }
    
    // 计算平均盈利和平均亏损
    double avg_profit = total_profit / profit_count;
    double avg_loss = total_loss / loss_count;
    
    if (avg_loss < EPSILON) {
        return 0.0;
    }
    
    // 盈亏比 = 平均盈利 / 平均亏损
    return avg_profit / avg_loss;
}

// ========== 交易指标：最大连续盈利/亏损次数 ==========
std::pair<int, int> PerformanceAnalyzer::calculate_max_consecutive(
    const std::vector<strategy::Trade>& trades
) {
    const double EPSILON = 1e-8;
    
    int max_consecutive_wins = 0;
    int max_consecutive_losses = 0;
    
    int current_wins = 0;
    int current_losses = 0;
    
    // 遍历所有交易，统计连续盈利和亏损
    for (const auto& trade : trades) {
        // 只统计平仓交易（有盈亏的交易）
        if (std::abs(trade.pnl) > EPSILON) {
            if (trade.pnl > EPSILON) {  // 盈利
                // 重置亏损计数，增加盈利计数
                current_losses = 0;
                current_wins++;
                max_consecutive_wins = std::max(max_consecutive_wins, current_wins);
            } else if (trade.pnl < -EPSILON) {  // 亏损
                // 重置盈利计数，增加亏损计数
                current_wins = 0;
                current_losses++;
                max_consecutive_losses = std::max(max_consecutive_losses, current_losses);
            }
        }
    }
    
    return std::make_pair(max_consecutive_wins, max_consecutive_losses);
}

// ========== 交易指标：平均持仓时间 ==========
double PerformanceAnalyzer::calculate_avg_holding_period(
    const std::vector<strategy::Trade>& trades
) {
    if (trades.size() < 2) {
        return 0.0;
    }
    
    // 需要配对买入和卖出交易来计算持仓时间
    // 简化处理：假设trades是按时间顺序排列的，且买入和卖出成对出现
    std::vector<double> holding_periods;
    
    for (size_t i = 0; i < trades.size(); i++) {
        if (trades[i].signal == strategy::Signal::BUY) {
            // 找到对应的卖出交易
            for (size_t j = i + 1; j < trades.size(); j++) {
                if (trades[j].signal == strategy::Signal::SELL && 
                    trades[j].symbol == trades[i].symbol) {
                    // 计算持仓时间（毫秒）
                    int64_t holding_ms = trades[j].timestamp - trades[i].timestamp;
                    if (holding_ms > 0) {
                        // 转换为天数（1天 = 24小时 * 3600秒 * 1000毫秒）
                        double holding_days = static_cast<double>(holding_ms) / (24.0 * 3600.0 * 1000.0);
                        holding_periods.push_back(holding_days);
                    }
                    break;  // 找到配对的卖出交易后退出内层循环
                }
            }
        }
    }
    
    if (holding_periods.empty()) {
        return 0.0;
    }
    
    // 计算平均持仓时间
    double sum = std::accumulate(holding_periods.begin(), holding_periods.end(), 0.0);
    return sum / holding_periods.size();
}

// ========== 交易指标：交易频率 ==========
double PerformanceAnalyzer::calculate_trade_frequency(
    int total_trades,
    Timestamp start_time,
    Timestamp end_time
) {
    if (total_trades <= 0) {
        return 0.0;
    }
    
    // 计算时间跨度（秒）
    int64_t time_span_seconds = end_time - start_time;
    if (time_span_seconds <= 0) {
        return 0.0;
    }
    
    // 转换为年数
    const double SECONDS_PER_YEAR = 365.25 * 24 * 3600;
    double years = static_cast<double>(time_span_seconds) / SECONDS_PER_YEAR;
    
    const double EPSILON = 1e-8;
    if (years < EPSILON) {
        return 0.0;
    }
    
    // 交易频率 = 总交易次数 / 年数
    return static_cast<double>(total_trades) / years;
}








}
}

