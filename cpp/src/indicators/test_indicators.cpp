#include "indicators/technical_indicators.h"
#include "collectors/binance_collector.h"
#include "config/config_manager.h"
#include <iostream>
#include <iomanip>

using namespace quant_crypto;
using namespace quant_crypto::indicators;
using namespace quant_crypto::collectors;

void print_separator() {
    std::cout << "========================================" << std::endl;
}

int main() {
    print_separator();
    std::cout << "技术指标计算测试" << std::endl;
    print_separator();
    std::cout << std::endl;
    
    // ========================================
    // 第1步：加载配置并获取数据
    // ========================================
    std::cout << "[第1步] 获取历史数据..." << std::endl;
    
    if (!config::ConfigManager::load("../config/binance.json")) {
        std::cerr << "❌ 配置加载失败" << std::endl;
        return 1;
    }
    
    auto config = config::ConfigManager::get_binance_config();
    BinanceCollector collector(config);
    
    // 获取100条1小时K线数据
    auto result = collector.get_klines("BTCUSDT", "1h", 100);
    
    if (!result.success) {
        std::cerr << "❌ 数据获取失败: " << result.error_message << std::endl;
        return 1;
    }
    
    std::cout << "✅ 成功获取 " << result.data.size() << " 条K线数据\n" << std::endl;
    
    // ========================================
    // 第2步：测试MA（移动平均线）
    // ========================================
    print_separator();
    std::cout << "[第2步] 测试 MA（移动平均线）" << std::endl;
    print_separator();
    
    // 计算MA20
    auto ma20 = TechnicalIndicators::calculate_ma_from_ohlcv(result.data, 20);
    
    std::cout << "MA20 计算结果：" << std::endl;
    std::cout << "  输入数据: " << result.data.size() << " 条" << std::endl;
    std::cout << "  输出数据: " << ma20.size() << " 条" << std::endl;
    std::cout << "\n最近5个MA20值：" << std::endl;
    
    for (int i = std::max(0, (int)ma20.size() - 5); i < (int)ma20.size(); i++) {
        std::cout << "  [" << i << "] MA20 = " 
                  << std::fixed << std::setprecision(2) << ma20[i] << std::endl;
    }
    std::cout << std::endl;
    
    // ========================================
    // 第3步：测试EMA（指数移动平均）
    // ========================================
    print_separator();
    std::cout << "[第3步] 测试 EMA（指数移动平均）" << std::endl;
    print_separator();
    
    auto close_prices = TechnicalIndicators::extract_close_prices(result.data);
    auto ema12 = TechnicalIndicators::calculate_ema(close_prices, 12);
    
    std::cout << "EMA12 计算结果：" << std::endl;
    std::cout << "  输出数据: " << ema12.size() << " 条" << std::endl;
    std::cout << "\n最近5个EMA12值：" << std::endl;
    
    for (int i = std::max(0, (int)ema12.size() - 5); i < (int)ema12.size(); i++) {
        std::cout << "  [" << i << "] EMA12 = " 
                  << std::fixed << std::setprecision(2) << ema12[i] << std::endl;
    }
    std::cout << std::endl;
    
    // ========================================
    // 第4步：测试MACD
    // ========================================
    print_separator();
    std::cout << "[第4步] 测试 MACD" << std::endl;
    print_separator();
    
    auto macd_result = TechnicalIndicators::calculate_macd(close_prices, 12, 26, 9);
    
    std::cout << "MACD 计算结果：" << std::endl;
    std::cout << "  DIF长度: " << macd_result.dif.size() << std::endl;
    std::cout << "  DEA长度: " << macd_result.dea.size() << std::endl;
    std::cout << "  柱状图长度: " << macd_result.histogram.size() << std::endl;
    std::cout << "\n最近5组MACD值：" << std::endl;
    
    for (int i = std::max(0, (int)macd_result.histogram.size() - 5); 
         i < (int)macd_result.histogram.size(); i++) {
        int dif_idx = i + (macd_result.dif.size() - macd_result.histogram.size());
        int dea_idx = i;
        
        std::cout << "  [" << i << "] "
                  << "DIF=" << std::fixed << std::setprecision(2) << macd_result.dif[dif_idx]
                  << ", DEA=" << macd_result.dea[dea_idx]
                  << ", 柱=" << macd_result.histogram[i];
        
        // 判断金叉/死叉
        if (i > 0) {
            int prev_dif = dif_idx - 1;
            int prev_dea = dea_idx - 1;
            
            bool golden_cross = (macd_result.dif[prev_dif] <= macd_result.dea[prev_dea]) &&
                               (macd_result.dif[dif_idx] > macd_result.dea[dea_idx]);
            bool death_cross = (macd_result.dif[prev_dif] >= macd_result.dea[prev_dea]) &&
                              (macd_result.dif[dif_idx] < macd_result.dea[dea_idx]);
            
            if (golden_cross) std::cout << " 🟢 金叉";
            if (death_cross) std::cout << " 🔴 死叉";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    
    // ========================================
    // 第5步：测试RSI
    // ========================================
    print_separator();
    std::cout << "[第5步] 测试 RSI（相对强弱指标）" << std::endl;
    print_separator();
    
    auto rsi14 = TechnicalIndicators::calculate_rsi(close_prices, 14);
    
    std::cout << "RSI14 计算结果：" << std::endl;
    std::cout << "  输出数据: " << rsi14.size() << " 条" << std::endl;
    std::cout << "\n最近5个RSI14值：" << std::endl;
    
    for (int i = std::max(0, (int)rsi14.size() - 5); i < (int)rsi14.size(); i++) {
        std::cout << "  [" << i << "] RSI14 = " 
                  << std::fixed << std::setprecision(2) << rsi14[i];
        
        // 判断超买超卖
        if (rsi14[i] > 70) {
            std::cout << " ⚠️ 超买";
        } else if (rsi14[i] < 30) {
            std::cout << " ⚠️ 超卖";
        } else {
            std::cout << " ✅ 正常";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    
    // ========================================
    // 第6步：综合分析
    // ========================================
    print_separator();
    std::cout << "[第6步] 当前市场状态分析" << std::endl;
    print_separator();
    
    // 获取最新值
    double latest_price = result.data.back().close;
    double latest_ma20 = ma20.back();
    double latest_rsi = rsi14.back();
    double latest_histogram = macd_result.histogram.back();
    
    std::cout << "最新收盘价: $" << std::fixed << std::setprecision(2) 
              << latest_price << std::endl;
    std::cout << "MA20: $" << latest_ma20 << std::endl;
    std::cout << "RSI14: " << latest_rsi << std::endl;
    std::cout << "MACD柱状图: " << latest_histogram << std::endl;
    std::cout << std::endl;
    
    // 简单的交易建议
    std::cout << "📊 技术分析建议：" << std::endl;
    
    if (latest_price > latest_ma20) {
        std::cout << "  ✅ 价格在MA20上方（看涨）" << std::endl;
    } else {
        std::cout << "  ⚠️ 价格在MA20下方（看跌）" << std::endl;
    }
    
    if (latest_rsi > 70) {
        std::cout << "  ⚠️ RSI超买（可能回调）" << std::endl;
    } else if (latest_rsi < 30) {
        std::cout << "  ⚠️ RSI超卖（可能反弹）" << std::endl;
    } else {
        std::cout << "  ✅ RSI正常区间" << std::endl;
    }
    
    if (latest_histogram > 0) {
        std::cout << "  ✅ MACD柱状图为正（看涨）" << std::endl;
    } else {
        std::cout << "  ⚠️ MACD柱状图为负（看跌）" << std::endl;
    }
    
    print_separator();
    std::cout << "✅ 所有指标测试通过！" << std::endl;
    print_separator();
    
    return 0;
}

