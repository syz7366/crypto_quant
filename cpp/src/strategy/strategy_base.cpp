#include "strategy/strategy_base.h"
#include <string>
#include<iostream>

namespace quant_crypto{
namespace strategy{

    // 设置初始资金
    void StrategyBase::on_init(double initial_capital) {
        capital_ = initial_capital;          // 当前资金
        initial_capital_ = initial_capital;  // 保存初始值（用于计算收益率）
    }

    // open_position- 开仓
    void StrategyBase::open_position(const std::string& symbol, double quantity, double price) {
        if(position_.has_position()) {
            std::cerr << "[StrategyBase] 已有持仓，无法开仓" << std::endl;
            return;
        }
        position_.symbol = symbol;
        position_.quantity = quantity;
        position_.avg_price = price;
        position_.current_price = price;
        position_.unrealized_pnl = 0;

        // // 添加交易记录 这里暂时不确定是否一定要加进去
        // Trade trade;
        // trade.timestamp = 20251130145900;
        // trade.symbol = symbol;
        // trade.signal = Signal::BUY;
        // trade.price = price;
        // trade.quantity = quantity;
        // trade.pnl = 0;
        // 扣除资金
        capital_ -= quantity * price;
    }

    // 清空持仓
    double StrategyBase::close_position(double price){
        if(!position_.has_position()) {
            std::cerr << "[StrategyBase] 没有持仓，无法平仓" << std::endl;
            return 0;   // 没有持仓，返回0 
        }

        // 计算盈亏
        double pnl = (price - position_.avg_price) * position_.quantity;
        // 回收资金
        capital_ += price * position_.quantity;

        // 清空持仓
        position_.quantity = 0;
        position_.avg_price = 0;
        position_.current_price = 0;
        position_.unrealized_pnl = 0;
        return pnl;  // 返回盈利
    }

    // 总权益： 资金+持仓市值
    double StrategyBase::get_total_equity() const {
        double equity = capital_;
        if(position_.has_position()) {
            equity += position_.quantity * position_.current_price;
        }
        return equity;
    }

    // 收益率
    double StrategyBase::get_total_return() const {
        if(initial_capital_ == 0)  return 0;
        return (get_total_equity() - initial_capital_) / initial_capital_ * 100.0;
    }

    // 更新持仓价格
    void StrategyBase::update_position_price(double current_price){
        position_.current_price = current_price;
        if (position_.has_position()) {
            position_.unrealized_pnl = (current_price - position_.avg_price) * position_.quantity;
        }
    }

    double StrategyBase::get_capital() const {
        return capital_;
    }
    
    Position StrategyBase::get_position() const {
        return position_;
    }
    
    void StrategyBase::add_trade(const Trade& trade) {
        trades_.push_back(trade);
    }

}
}