#include "cleaners/data_cleaner.h"
#include "collectors/binance_collector.h"
#include "storage/kline_storage.h"
#include "config/config_manager.h"
#include "common/types.h"
#include <iostream>
#include <iomanip>

using namespace quant_crypto;
using namespace quant_crypto::collectors;
using namespace quant_crypto::cleaners;

/**
 * @brief 打印OHLCV数据及其质量状态
 */
void print_ohlcv(const OHLCV& ohlcv, int index) {
    std::cout << "K线 " << index << ":" << std::endl;
    std::cout << "  时间戳: " << ohlcv.timestamp << std::endl;
    std::cout << "  交易对: " << ohlcv.symbol << std::endl;
    std::cout << "  OHLC: " << std::fixed << std::setprecision(2) 
              << ohlcv.open << " / " << ohlcv.high << " / " 
              << ohlcv.low << " / " << ohlcv.close << std::endl;
    std::cout << "  成交量: " << ohlcv.volume << std::endl;
    
    // 数据质量显示
    std::string quality_str;
    switch(ohlcv.quality) {
        case DataQuality::GOOD: quality_str = "✅ 良好"; break;
        case DataQuality::SUSPICIOUS: quality_str = "⚠️  可疑"; break;
        case DataQuality::BAD: quality_str = "❌ 异常"; break;
        case DataQuality::MISSING: quality_str = "🔍 缺失"; break;
        default: quality_str = "❓ 未知"; break;
    }
    std::cout << "  数据质量: " << quality_str << std::endl;
    std::cout << std::endl;
}

/**
 * @brief 统计数据质量分布
 */
void print_quality_stats(const std::vector<OHLCV>& data) {
    int good = 0, suspicious = 0, bad = 0, missing = 0;
    
    for (const auto& ohlcv : data) {
        switch(ohlcv.quality) {
            case DataQuality::GOOD: good++; break;
            case DataQuality::SUSPICIOUS: suspicious++; break;
            case DataQuality::BAD: bad++; break;
            case DataQuality::MISSING: missing++; break;
        }
    }
    
    std::cout << "\n📊 数据质量统计：" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "✅ 良好: " << good << " 条 (" 
              << std::fixed << std::setprecision(1) 
              << (100.0 * good / data.size()) << "%)" << std::endl;
    std::cout << "⚠️  可疑: " << suspicious << " 条 (" 
              << (100.0 * suspicious / data.size()) << "%)" << std::endl;
    std::cout << "❌ 异常: " << bad << " 条 (" 
              << (100.0 * bad / data.size()) << "%)" << std::endl;
    std::cout << "🔍 缺失: " << missing << " 条 (" 
              << (100.0 * missing / data.size()) << "%)" << std::endl;
    std::cout << "总计: " << data.size() << " 条" << std::endl;
    std::cout << "----------------------------------------\n" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "数据清洗模块测试" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // ========================================
    // 第1步：加载配置
    // ========================================
    std::cout << "📝 第1步：加载配置文件..." << std::endl;
    if (!config::ConfigManager::load("../config/binance.json")) {
        std::cerr << "❌ 配置文件加载失败！程序退出。" << std::endl;
        return 1;
    }
    auto config = config::ConfigManager::get_binance_config();
    std::cout << "✅ 配置加载成功\n" << std::endl;
    
    // ========================================
    // 第2步：获取原始数据
    // ========================================
    std::cout << "📝 第2步：获取原始K线数据..." << std::endl;
    BinanceCollector collector(config);
    
    // 获取更多数据以便测试清洗效果
    auto result = collector.get_klines("BTCUSDT", "1h", 20);
    
    if (!result.success) {
        std::cerr << "❌ 获取数据失败: " << result.error_message << std::endl;
        return 1;
    }
    
    std::cout << "✅ 成功获取 " << result.data.size() << " 条原始数据\n" << std::endl;
    
    // ========================================
    // 第3步：创建数据清洗器并添加规则
    // ========================================
    std::cout << "📝 第3步：创建数据清洗器..." << std::endl;
    DataCleaner cleaner;
    
    // 添加价格跳变检测规则（阈值30%）
    cleaner.add_rule(std::make_shared<PriceJumpRule>(0.30));
    
    // 添加成交量异常检测规则（阈值5倍）
    cleaner.add_rule(std::make_shared<VolumeAnomalyRule>(5.0));
    
    std::cout << "✅ 清洗器初始化完成\n" << std::endl;
    
    // ========================================
    // 第4步：执行数据清洗
    // ========================================
    std::cout << "📝 第4步：执行数据清洗..." << std::endl;
    std::cout << "应用规则：" << std::endl;
    std::cout << "  - 价格合理性检查" << std::endl;
    std::cout << "  - OHLC关系检查" << std::endl;
    std::cout << "  - 价格跳变检测（阈值30%）" << std::endl;
    std::cout << "  - 成交量异常检测（阈值5倍）\n" << std::endl;
    
    // 清洗数据
    auto cleaned_data = cleaner.clean_ohlcv_batch(result.data);
    
    std::cout << "✅ 清洗完成！" << std::endl;
    std::cout << "原始数据: " << result.data.size() << " 条" << std::endl;
    std::cout << "清洗后: " << cleaned_data.size() << " 条" << std::endl;
    std::cout << "过滤掉: " << (result.data.size() - cleaned_data.size()) << " 条\n" << std::endl;
    
    // ========================================
    // 第5步：显示清洗前后对比
    // ========================================
    std::cout << "\n========================================" << std::endl;
    std::cout << "📊 数据质量对比" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // 显示原始数据质量统计
    std::cout << "【原始数据】" << std::endl;
    print_quality_stats(result.data);
    
    // 显示清洗后数据质量统计
    std::cout << "【清洗后数据】" << std::endl;
    print_quality_stats(cleaned_data);
    
    // ========================================
    // 第6步：显示前5条数据详情
    // ========================================
    std::cout << "\n========================================" << std::endl;
    std::cout << "📋 清洗后数据示例（前5条）" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    int count = std::min(5, (int)cleaned_data.size());
    for (int i = 0; i < count; i++) {
        print_ohlcv(cleaned_data[i], i + 1);
    }
    
    // ========================================
    // 第7步：去重处理
    // ========================================
    std::cout << "\n========================================" << std::endl;
    std::cout << "📝 第7步：去重处理..." << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    auto deduped_data = DataCleaner::deduplicate(cleaned_data);
    std::cout << "清洗后数据: " << cleaned_data.size() << " 条" << std::endl;
    std::cout << "去重后数据: " << deduped_data.size() << " 条" << std::endl;
    std::cout << "去除重复: " << (cleaned_data.size() - deduped_data.size()) << " 条\n" << std::endl;
    
    // ========================================
    // 第8步：保存清洗后的数据
    // ========================================
    std::cout << "\n========================================" << std::endl;
    std::cout << "📝 第8步：保存清洗后的数据..." << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    storage::KlineStorage storage(config.data_dir);
    bool save_success = storage.save_ohlcv("BTCUSDT_cleaned", "1h", deduped_data);
    
    if (save_success) {
        std::cout << "✅ 清洗后的数据已保存！" << std::endl;
        std::cout << "文件位置: " << config.data_dir << "/BTCUSDT_cleaned_1h_YYYYMMDD.csv" << std::endl;
    } else {
        std::cerr << "❌ 数据保存失败！" << std::endl;
    }
    
    // ========================================
    // 总结
    // ========================================
    std::cout << "\n========================================" << std::endl;
    std::cout << "✅ 数据清洗测试完成！" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "\n📊 清洗流程总结：" << std::endl;
    std::cout << "  1. 原始数据: " << result.data.size() << " 条" << std::endl;
    std::cout << "  2. 清洗过滤: " << (result.data.size() - cleaned_data.size()) << " 条" << std::endl;
    std::cout << "  3. 去重处理: " << (cleaned_data.size() - deduped_data.size()) << " 条" << std::endl;
    std::cout << "  4. 最终数据: " << deduped_data.size() << " 条" << std::endl;
    std::cout << "  5. 数据保留率: " << std::fixed << std::setprecision(1) 
              << (100.0 * deduped_data.size() / result.data.size()) << "%" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}

