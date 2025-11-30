#pragma once
#include "strategy/strategy_base.h"
#include "indicators/technical_indicators.h"
#include <deque>

namespace quant_crypto{
namespace strategy{

// 策略配置
struct MACrossConfig{
    int fast_period;
    int slow_period;
    double position_size;

    // 构造函数（默认值：MA5， MA20，50%仓位）
    MACrossConfig() : fast_period(5), slow_period(20), position_size(0.5) {}
};

//MA 交叉策略类
class MACrossStrategy:public StrategyBase{
public:
    //构造函数
    explicit MACrossStrategy(const MACrossConfig& config=MACrossConfig());

    // 虚函数
    void on_bar(const OHLCV& bar) override;   // 这个方法具体的作用是什么？
    Signal generate_signal() override;
    std::string get_name() const override;

    // 添加getter方法
    double get_fast_ma() const;   // 获取当前快线值
    double get_slow_ma() const;   // 获取当前慢线值

private:
    // 成员变量
    MACrossConfig config_;
    std::deque<double> price_history_;
    std::vector<double> fast_ma_;
    std::vector<double> slow_ma_;
    Signal last_signal_;
    OHLCV current_bar_;

    // 私有方法
    void update_ma();
    Signal detect_cross();
};


}
}






