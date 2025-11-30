#pragma once
#include "common/types.h"
#include<vector>
#include<string>

namespace quant_crypto{
namespace strategy{

enum class Signal{
    BUY, // 买入
    SELL, // 卖出
    HOLD, // 持有
    NONE // 无信号
};

// 持仓信息
struct Position{
    std::string symbol;  // 交易对
    double quantity;   // 持仓数量
    double avg_price;  // 平均成本
    double current_price;  // 当前价格
    double unrealized_pnl;   // 未实现盈亏

    Position() : symbol(""), quantity(0.0), avg_price(0.0), current_price(0.0), unrealized_pnl(0.0) {}

    // 添加一个方法： 是否有持仓
    bool has_position() const {
        return quantity != 0;
    }
};

struct Trade{
    quant_crypto::Timestamp timestamp;
    std::string symbol;
    Signal signal;   // 买入/卖出
    double price;  // 成交价格
    double quantity;  // 成交数量
    double pnl;   // 盈亏(平仓时)

    Trade() : timestamp(0), symbol(""), signal(Signal::NONE), price(0.0), quantity(0.0), pnl(0.0) {}
};

class StrategyBase{
public:
    // virtual StrategyBase() = 0;
    virtual ~StrategyBase() = default;

    // 纯虚函数
    virtual void on_bar(const OHLCV& bar)=0;
    virtual Signal generate_signal() = 0;
    virtual std::string get_name() const = 0;

    // 设置参数
    // virtual void set_param(const std::string& name, const std::string& value) = 0;
    
    // 通用方法（基类提供）
    void on_init(double initial_capital);
    void open_position(const std::string& symbol, double quantity, double price);
    double close_position(double price);    // 返回平仓亏损
    double get_capital() const;

    double get_total_equity() const;      // 获取总权益
    double get_total_return() const;      // 获取收益率
    Position get_position() const;        // 获取持仓
    void update_position_price(double current_price);  // 更新持仓价格
    void add_trade(const Trade& trade);
    
protected:
    Position position_;
    double capital_;
    double initial_capital_;
    std::vector<Trade> trades_;

};
}
}
