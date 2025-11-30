#include "strategy/ma_cross_strategy.h"
#include<iostream>

namespace quant_crypto{
namespace strategy{
    void MACrossStrategy::on_bar(const OHLCV& bar) {
        // 1. 保存当前K线
        current_bar_ = bar;

        // 2. 添加收盘价到历史—— 
        price_history_.push_back(bar.close);

        //3. 如果历史太长，删除最旧的
        if(price_history_.size() > config_.slow_period +10){
            price_history_.pop_front();
        }

        // 4.更新MA值
        update_ma();

        //5. 如果有持仓，更新当前价格
        if(position_.has_position()){
            update_position_price(bar.close);
        }
    }

    MACrossStrategy::MACrossStrategy(const MACrossConfig& config) : config_(config),last_signal_(Signal::NONE) {
        //  可以添加参数验证 
        if(config_.fast_period >= config_.slow_period){
            std::cerr << " 警告：快线周期应小于慢线周期" << std::endl;
        }
    }

    std::string MACrossStrategy::get_name() const {
        return "MA Cross Strategy";
    }

    double MACrossStrategy::get_fast_ma() const {
        return fast_ma_.empty() ? 0 : fast_ma_.back();
    }
    
    double MACrossStrategy::get_slow_ma() const {
        return slow_ma_.empty() ? 0 : slow_ma_.back();
    }


    void MACrossStrategy::update_ma() {
        // 1. 把deque转成vector（因为技术指标函数需要vector）
        std::vector<double> prices(price_history_.begin(), price_history_.end());
        
        // 2. 计算快线（MA5）
        if (prices.size() >= config_.fast_period) {
            auto ma_result = indicators::TechnicalIndicators::calculate_ma(prices, config_.fast_period);
            if (!ma_result.empty()) {
                fast_ma_.push_back(ma_result.back());  // 只保存最新值
                if (fast_ma_.size() > 2) fast_ma_.erase(fast_ma_.begin());  // 只保留2个
            }
        }
        
        // 3. 计算慢线（MA20）- 同样逻辑
        if (prices.size() >= config_.slow_period) {
            auto ma_result = indicators::TechnicalIndicators::calculate_ma(prices, config_.slow_period);
            if (!ma_result.empty()) {
                slow_ma_.push_back(ma_result.back());
                if (slow_ma_.size() > 2) slow_ma_.erase(slow_ma_.begin());
            }
        }
    }

    Signal MACrossStrategy::detect_cross(){
        // 1. 检查是否有足够数据（需要至少2个MA值）
        if(fast_ma_.size() < 2 || slow_ma_.size() < 2) return Signal::NONE;

        // 2. 获取值
        double fast_curr = fast_ma_[1];   //当前快线
        double fast_prev = fast_ma_[0];   // 前一个快线
        double slow_curr = slow_ma_[1];   //当前慢线 
        double slow_prev = slow_ma_[0];   // 前一个慢线

        //3. 判断金叉
        if(fast_prev <= slow_prev && fast_curr > slow_curr){
            return Signal::BUY;
        }

        //4. 判断死叉
        if(fast_prev >= slow_prev && fast_curr < slow_curr){
            return Signal::SELL;
        }
        return Signal::NONE;
    }

    Signal MACrossStrategy::generate_signal(){
        //1. 检查数据量
        if(price_history_.size()<config_.slow_period){
            return Signal::NONE;
        }
        //2. 检查交叉
        Signal signal = detect_cross();

        //3. 过滤信号(已有持仓不买入，没有持仓不卖出)
        if(signal == Signal::BUY && position_.has_position()) return Signal::HOLD;
        if(signal == Signal::SELL && !position_.has_position()) return Signal::HOLD;
    
        // 4. 防止重复信号
        if(signal == last_signal_) return Signal::HOLD;

        //5. 更新信号状态
        if(signal != Signal::NONE){
            last_signal_ = signal;
        }
        return signal;
    }


}
}

