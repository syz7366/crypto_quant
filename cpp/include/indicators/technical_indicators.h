#pragma once

#include "common/types.h"
#include <vector>
#include <stdexcept>

namespace quant_crypto{
namespace indicators{


/**
 * @class TechnicalIndicators
 * @brief 技术指标计算类
 * 
 * 提供常用技术指标的计算方法
 */
class TechnicalIndicators{
public:
    /**
     * @brief 计算简单移动平均线（SMA/MA）
     * @param prices 价格序列
     * @param period 周期（如20表示20日均线）
     * @return MA值序列（长度 = prices.size() - period + 1）
     * 
     * 示例：
     *   prices = [100, 102, 104, 106, 108]
     *   MA(3) = [102, 104, 106]
     */
    static std::vector<double> calculate_ma(
        const std::vector<double>& prices, 
        int period
    );

    // 从OHLCV数据计算MA
    static std::vector<double> calculate_ma_from_ohlcv(
        const std::vector<OHLCV>& ohlcv,
        int period
    );

    // 计算指数移动平均线
    static std::vector<double> calculate_ema(
        const std::vector<double>& prices,
        int period
    );

    // 计算MACD指标
    struct MACDResult {
        std::vector<double> dif;        // MACD线（快线-慢线）
        std::vector<double> dea;        // 信号线（DIF的EMA）
        std::vector<double> histogram;  // 柱状图（DIF-DEA）
    };

    static MACDResult calculate_macd(
        const std::vector<double>& prices,
        int fast_period = 12,
        int slow_period = 26,
        int signal_period = 9
    );

    // 计算 RSI指标（相对强弱指标）
    static std::vector<double> calculate_rsi(
        const std::vector<double>& prices,
        int period = 14
    );

    // 从 OHLCV 提取收盘价序列
    static std::vector<double> extract_close_prices(
        const std::vector<OHLCV>& ohlcv_list
    );

    // 从OHLCV提取开盘价格序列
    static std::vector<double> extract_open_prices(
        const std::vector<OHLCV>& ohlcv_list
    );

    /**
     * @brief 从OHLCV提取最高价序列
     */
     static std::vector<double> extract_high_prices(
        const std::vector<OHLCV>& ohlcv_list
    );
    
    /**
     * @brief 从OHLCV提取最低价序列
     */
    static std::vector<double> extract_low_prices(
        const std::vector<OHLCV>& ohlcv_list
    );
    
    /**
     * @brief 从OHLCV提取成交量序列
     */
    static std::vector<double> extract_volumes(
        const std::vector<OHLCV>& ohlcv_list
    );

private:
    /**
     * @brief 验证参数有效性
     */
    static void validate_params(
        const std::vector<double>& data,
        int period,
        const std::string& indicator_name
    );
};



}


}


