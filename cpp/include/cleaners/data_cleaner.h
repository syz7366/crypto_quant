#pragma once

#include "common/types.h"
#include <vector>
#include <memory>
#include <functional>

namespace quant_crypto {
namespace cleaners {

/**
 * @brief 数据清洗规则接口
 */
class CleaningRule {
public:
    virtual ~CleaningRule() = default;
    
    /**
     * @brief 对OHLCV数据应用清洗规则
     * @param data OHLCV数据
     * @return 如果数据应该被保留返回true，否则返回false
     */
    virtual bool apply(OHLCV& data) const = 0;
    
    /**
     * @brief 获取规则名称
     */
    virtual std::string get_name() const = 0;
};

/**
 * @brief 数据清洗器
 * 
 * 负责检测和处理异常数据
 */
class DataCleaner {
public:
    DataCleaner();
    ~DataCleaner() = default;

    /**
     * @brief 添加清洗规则
     * @param rule 清洗规则
     */
    void add_rule(std::shared_ptr<CleaningRule> rule);

    /**
     * @brief 清洗OHLCV数据
     * @param data OHLCV数据
     * @return 清洗后的数据质量标志
     */
    DataQuality clean_ohlcv(OHLCV& data) const;

    /**
     * @brief 批量清洗OHLCV数据
     * @param data_list OHLCV数据列表
     * @return 清洗后保留的数据
     */
    std::vector<OHLCV> clean_ohlcv_batch(const std::vector<OHLCV>& data_list) const;

    /**
     * @brief 清洗Tick数据
     * @param data Tick数据
     * @return 清洗后的数据质量标志
     */
    DataQuality clean_tick(Tick& data) const;

    /**
     * @brief 清洗OrderBook数据
     * @param data OrderBook数据
     * @return 清洗后的数据质量标志
     */
    DataQuality clean_orderbook(OrderBook& data) const;

    /**
     * @brief 检测价格异常跳变
     * @param current 当前价格
     * @param previous 前一个价格
     * @param threshold 异常阈值（百分比，如0.5表示50%）
     * @return 如果异常返回true
     */
    static bool detect_price_jump(Price current, Price previous, double threshold = 0.5);

    /**
     * @brief 检测成交量异常
     * @param volume 当前成交量
     * @param avg_volume 平均成交量
     * @param threshold 异常阈值（倍数，如10表示10倍）
     * @return 如果异常返回true
     */
    static bool detect_volume_anomaly(Volume volume, Volume avg_volume, double threshold = 10.0);

    /**
     * @brief 检测时间戳是否连续
     * @param current_ts 当前时间戳
     * @param previous_ts 前一个时间戳
     * @param expected_interval 期望间隔（毫秒）
     * @param tolerance 容差（毫秒）
     * @return 如果连续返回true
     */
    static bool is_timestamp_continuous(
        Timestamp current_ts,
        Timestamp previous_ts,
        int64_t expected_interval,
        int64_t tolerance = 1000);

    /**
     * @brief 去除重复数据
     * @param data_list 数据列表
     * @return 去重后的数据列表
     */
    static std::vector<OHLCV> deduplicate(const std::vector<OHLCV>& data_list);

    /**
     * @brief 填补缺失数据
     * @param data_list 数据列表
     * @param timeframe 时间周期
     * @param method 填补方法（"forward", "backward", "interpolate"）
     * @return 填补后的数据列表
     */
    static std::vector<OHLCV> fill_missing(
        const std::vector<OHLCV>& data_list,
        Timeframe timeframe,
        const std::string& method = "forward");

private:
    std::vector<std::shared_ptr<CleaningRule>> rules_;
};

// 预定义清洗规则

/**
 * @brief 价格合理性检查规则
 */
class PriceValidityRule : public CleaningRule {
public:
    bool apply(OHLCV& data) const override;
    std::string get_name() const override { return "PriceValidityRule"; }
};

/**
 * @brief 价格跳变检测规则
 */
class PriceJumpRule : public CleaningRule {
public:
    explicit PriceJumpRule(double threshold = 0.5) : threshold_(threshold) {}
    bool apply(OHLCV& data) const override;
    std::string get_name() const override { return "PriceJumpRule"; }
    
private:
    double threshold_;
    mutable Price last_close_ = 0.0;
};

/**
 * @brief 成交量异常检测规则
 */
class VolumeAnomalyRule : public CleaningRule {
public:
    explicit VolumeAnomalyRule(double threshold = 10.0) : threshold_(threshold) {}
    bool apply(OHLCV& data) const override;
    std::string get_name() const override { return "VolumeAnomalyRule"; }
    
private:
    double threshold_;
    mutable std::vector<Volume> volume_history_;
    static constexpr size_t HISTORY_SIZE = 100;
};

/**
 * @brief OHLC关系检查规则
 */
class OHLCRelationRule : public CleaningRule {
public:
    bool apply(OHLCV& data) const override;
    std::string get_name() const override { return "OHLCRelationRule"; }
};

} // namespace cleaners
} // namespace quant_crypto

