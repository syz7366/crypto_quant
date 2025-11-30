#include "indicators/technical_indicators.h"
#include <numeric>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace quant_crypto {
namespace indicators{

void TechnicalIndicators::validate_params(
    const std::vector<double>& data,
    int period,
    const std::string& indicator_name
) {
    if (data.empty()) {
        throw std::invalid_argument("数据序列不能为空");
    }

    if(period <=0){
        throw std::invalid_argument("周期必须大于0");
    }
    if(static_cast<int>(data.size()) < period){
        throw std::invalid_argument(
            indicator_name + ": 数据长度(" + std::to_string(data.size()) + 
            ")小于周期(" + std::to_string(period) + ")"
        );
    }
}

// 移动平均线
std::vector<double> TechnicalIndicators::calculate_ma(
    const std::vector<double>& prices,
    int period
){
    validate_params(prices, period, "MA");
    std::vector<double> ma_values;

    int data_size = prices.size();
    // 从period个数据开始计算
    for(int i = period -1 ; i<data_size;i++){
        double sum = 0.0;

        // 计算period个价格的平均值
        for(int j=0; j<period;j++){
            sum += prices[i-j];
        }
        double ma = sum / period;
        ma_values.push_back(ma);
    }
    return ma_values;
}

std::vector<double> TechnicalIndicators::calculate_ma_from_ohlcv(
    const std::vector<OHLCV>& ohlcv_list,
    int period
){
    // 提取收盘价
    auto close_prices = extract_close_prices(ohlcv_list);
    // 计算MA
    return calculate_ma(close_prices, period);
}

// EMA 指数移动平均线
std::vector<double> TechnicalIndicators::calculate_ema(
    const std::vector<double>& prices,
    int period
){
    validate_params(prices, period, "EMA");
    std::vector<double> ema_values;

    // 计算平滑系数
    double alpha = 2.0 / (period +1);

    // 第一个EMA值使用SMA
    double sum = 0.0;
    for(int i=0; i< period ;i++){
        sum += prices[i];
    }
    double ema = sum / period;
    ema_values.push_back(ema);

    // 后续使用EMA公式
    for(int i=period; i<prices.size();i++){
        ema = alpha * prices[i] + (1-alpha) * ema;
        ema_values.push_back(ema);
    }
    return ema_values;
}

// MACD
// ============================================================

TechnicalIndicators::MACDResult TechnicalIndicators::calculate_macd(
    const std::vector<double>& prices,
    int fast_period,
    int slow_period,
    int signal_period) {
    
    validate_params(prices, slow_period, "MACD");
    
    MACDResult result;
    
    // 1. 计算快线EMA和慢线EMA
    auto fast_ema = calculate_ema(prices, fast_period);
    auto slow_ema = calculate_ema(prices, slow_period);
    
    // 2. 计算DIF（快线-慢线）
    // 注意：slow_ema比fast_ema短，需要对齐
    int offset = fast_period - slow_period;  // 负数
    for (size_t i = 0; i < slow_ema.size(); i++) {
        double dif = fast_ema[i - offset] - slow_ema[i];
        result.dif.push_back(dif);
    }
    
    // 3. 计算DEA（DIF的EMA）
    if (result.dif.size() >= static_cast<size_t>(signal_period)) {
        result.dea = calculate_ema(result.dif, signal_period);
    }
    
    // 4. 计算柱状图（DIF-DEA）
    // DEA比DIF短，需要对齐
    int dea_offset = signal_period - 1;
    for (size_t i = 0; i < result.dea.size(); i++) {
        double histogram = result.dif[i + dea_offset] - result.dea[i];
        result.histogram.push_back(histogram);
    }
    return result;
}

// RSI（相对强弱指标）
// ============================================================
std::vector<double> TechnicalIndicators::calculate_rsi(
    const std::vector<double>& prices,
    int period) {
    
    validate_params(prices, period + 1, "RSI");  // 需要period+1个数据
    
    std::vector<double> rsi_values;
    
    // 计算价格变化
    std::vector<double> gains;
    std::vector<double> losses;
    
    for (size_t i = 1; i < prices.size(); i++) {
        double change = prices[i] - prices[i-1];
        gains.push_back(change > 0 ? change : 0);
        losses.push_back(change < 0 ? -change : 0);
    }
    
    // 计算初始的平均涨幅和跌幅
    double avg_gain = 0.0;
    double avg_loss = 0.0;
    
    for (int i = 0; i < period; i++) {
        avg_gain += gains[i];
        avg_loss += losses[i];
    }
    avg_gain /= period;
    avg_loss /= period;
    
    // 计算第一个RSI
    double rs = (avg_loss == 0) ? 100.0 : avg_gain / avg_loss;
    double rsi = 100.0 - (100.0 / (1.0 + rs));
    rsi_values.push_back(rsi);
    
    // 使用平滑方法计算后续RSI
    for (size_t i = period; i < gains.size(); i++) {
        avg_gain = (avg_gain * (period - 1) + gains[i]) / period;
        avg_loss = (avg_loss * (period - 1) + losses[i]) / period;
        
        rs = (avg_loss == 0) ? 100.0 : avg_gain / avg_loss;
        rsi = 100.0 - (100.0 / (1.0 + rs));
        rsi_values.push_back(rsi);
    }
    
    return rsi_values;
}


// 数据提取工具函数
std::vector<double> TechnicalIndicators::extract_close_prices(
    const std::vector<OHLCV>& ohlcv_list
){
    std::vector<double> prices;
    prices.reserve(ohlcv_list.size());

    for(const auto& ohlcv : ohlcv_list){
        prices.push_back(ohlcv.open);
    }
    return prices;
}


std::vector<double> TechnicalIndicators::extract_high_prices(
    const std::vector<OHLCV>& ohlcv_list) {
    
    std::vector<double> prices;
    prices.reserve(ohlcv_list.size());
    
    for (const auto& ohlcv : ohlcv_list) {
        prices.push_back(ohlcv.high);
    }
    
    return prices;
}

std::vector<double> TechnicalIndicators::extract_low_prices(
    const std::vector<OHLCV>& ohlcv_list) {
    
    std::vector<double> prices;
    prices.reserve(ohlcv_list.size());
    
    for (const auto& ohlcv : ohlcv_list) {
        prices.push_back(ohlcv.low);
    }
    
    return prices;
}

std::vector<double> TechnicalIndicators::extract_volumes(
    const std::vector<OHLCV>& ohlcv_list) {
    
    std::vector<double> volumes;
    volumes.reserve(ohlcv_list.size());
    
    for (const auto& ohlcv : ohlcv_list) {
        volumes.push_back(ohlcv.volume);
    }
    
    return volumes;
}


}
}


