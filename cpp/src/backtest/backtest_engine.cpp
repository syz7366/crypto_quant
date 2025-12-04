#include "backtest/backtest_engine.h"
#include <iostream>

namespace quant_crypto {
namespace backtest {    

BacktestEngine::BacktestEngine(const BacktestConfig& config)
    : config_(config),strategy_(nullptr) {
        // 初始化 result_
        result_.initial_capital = config_.initial_capital;
    }

    void BacktestEngine::set_strategy(strategy::StrategyBase* strategy){
        strategy_ = strategy;
    }
    void BacktestEngine::set_data(const std::vector<OHLCV>& data){
        data_ = data;
    }

    void BacktestEngine::run(){
        //1. 验证
        if(!strategy_ || data_.empty()){
            std::cerr << "策略或数据为空" << std::endl;
            return;
        }

        // 2. 初始化策略
        strategy_->on_init(config_.initial_capital);

        // ============ 新增：初始化权益曲线 =========
        // 记录初始权益
        result_.equity_curve.clear();
        result_.timestamps.clear();
        result_.equity_curve.push_back(config_.initial_capital);
        if(!data_.empty()){
            result_.timestamps.push_back(data_[0].timestamp);
        }

        // 3. 回测循环
        for(const auto& bar:data_){
            //3.1 喂数据给策略
            strategy_->on_bar(bar);

            //3.2 生成信号
            auto signal = strategy_->generate_signal();

            //3.3 处理信号
            if(signal != strategy::Signal::HOLD && signal != strategy::Signal::NONE){
                process_signal(signal,bar);
            }
            // ============ 记录每个Bar结束时的权益 =========
            // 更新持仓价格（用于计算未实现盈亏）         那么这里为什么之前不更新持仓价格呢，要到现在做性能评测了才做更新
            if (strategy_->get_position().quantity > 0){
                strategy_->update_position_price(bar.close);
            }

            // 记录当前总权益
            double current_equity = strategy_->get_total_equity();
            result_.equity_curve.push_back(current_equity);
            result_.timestamps.push_back(bar.timestamp);
        }
        // 4. 汇总结果
        result_.final_capital = strategy_->get_capital();
        result_.final_equity = strategy_->get_total_equity();
        result_.total_return = strategy_->get_total_return();
    }

    void BacktestEngine::process_signal(strategy::Signal signal,const OHLCV& bar){
        if (signal == strategy::Signal::BUY) {
            // 计算买入金额（使用当前资金的一定比例，如50%）
            double capital = strategy_->get_capital();
            double buy_amount = capital * 0.5;  // 50%仓位
            
            // 计算滑点后的实际价格
            double slippage = calculate_slippage(bar.close);
            double actual_price = bar.close + slippage;  // 买入时价格上涨
            
            // 计算手续费
            double commission = calculate_commission(buy_amount);
            
            // 计算实际能买的数量
            double quantity = (buy_amount - commission) / actual_price;
            
            // 开仓
            strategy_->open_position(bar.symbol, quantity, actual_price);
            
            // 记录交易
            strategy::Trade trade;
            trade.timestamp = bar.timestamp;
            trade.symbol = bar.symbol;
            trade.signal = strategy::Signal::BUY;
            trade.price = actual_price;
            trade.quantity = quantity;
            trade.pnl = 0;
            strategy_->add_trade(trade);
            result_.trades.push_back(trade);
            result_.total_trades++;
        } else if (signal == strategy::Signal::SELL) {
            // 计算滑点后的实际价格
            double slippage = calculate_slippage(bar.close);
            double actual_price = bar.close - slippage;  // 卖出时价格下跌
            
            // 保存持仓数量（在平仓前）
            double position_quantity = strategy_->get_position().quantity;

            // 平仓并获取盈亏
            double pnl = strategy_->close_position(actual_price);
            
            // 计算手续费
            double sell_amount = actual_price * position_quantity;
            double commission = calculate_commission(sell_amount);
            
            // 从盈亏中扣除手续费
            pnl -= commission;
            
            // 记录交易
            strategy::Trade trade;
            trade.timestamp = bar.timestamp;
            trade.symbol = bar.symbol;
            trade.signal = strategy::Signal::SELL;
            trade.price = actual_price;
            trade.quantity = 0;
            trade.pnl = pnl;
            strategy_->add_trade(trade);
            result_.trades.push_back(trade);
            result_.total_trades++;
            
            // 统计胜率
            if (pnl > 0) {
                result_.winning_trades++;
            } else if (pnl < 0) {
                result_.losing_trades++;
            }
        }
    }

    double BacktestEngine::calculate_commission(double amount) {
        return amount * config_.commission_rate;
    }
    
    double BacktestEngine::calculate_slippage(double price) {
        return price * config_.slippage_rate;
    }

    BacktestResult BacktestEngine::get_result() const {
        return result_;
    }

    

}
}



