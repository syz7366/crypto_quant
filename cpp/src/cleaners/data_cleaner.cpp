#include "cleaners/data_cleaner.h"
#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <numeric>

namespace quant_crypto {
namespace cleaners {

DataCleaner::DataCleaner() {
    // 默认添加一些基础清洗规则
    add_rule(std::make_shared<PriceValidityRule>());
    add_rule(std::make_shared<OHLCRelationRule>());
}

void DataCleaner::add_rule(std::shared_ptr<CleaningRule> rule) {
    rules_.push_back(rule);
}

DataQuality DataCleaner::clean_ohlcv(OHLCV& data) const {
    for (const auto& rule : rules_) {
        if (!rule->apply(data)) {
            return DataQuality::BAD;
        }
    }
    return DataQuality::GOOD;

}

std::vector<OHLCV> DataCleaner::clean_ohlcv_batch(const std::vector<OHLCV>& data_list) const {
    std::vector<OHLCV> cleaned_data;
    cleaned_data.reserve(data_list.size());
    
    for (auto data : data_list) {
        DataQuality quality = clean_ohlcv(data);
        if (quality == DataQuality::GOOD) {
            cleaned_data.push_back(data);
        }
    }
    
    return cleaned_data;
}

DataQuality DataCleaner::clean_tick(Tick& data) const {
    if (data.timestamp <= 0 || data.price <= 0 || data.volume < 0) {
        return DataQuality::BAD;
    }
    return DataQuality::GOOD;
}

DataQuality DataCleaner::clean_orderbook(OrderBook& data) const {
    if (data.timestamp <= 0) {
        return DataQuality::BAD;
    }
    if (data.bids.empty() || data.asks.empty()) {
        return DataQuality::BAD;
    }
    return DataQuality::GOOD;
}

bool DataCleaner::detect_price_jump(Price current, Price previous, double threshold) {
    if (previous <= 0) return false;
    double change = std::abs((current - previous) / previous);
    return change > threshold;
}

bool DataCleaner::detect_volume_anomaly(Volume volume, Volume avg_volume, double threshold) {
    if (avg_volume <= 0) return false;
    return volume > avg_volume * threshold;
}

bool DataCleaner::is_timestamp_continuous(
    Timestamp current_ts,
    Timestamp previous_ts,
    int64_t expected_interval,
    int64_t tolerance) {
    
    int64_t actual_interval = current_ts - previous_ts;
    int64_t diff = std::abs(actual_interval - expected_interval);
    return diff <= tolerance;
}

std::vector<OHLCV> DataCleaner::deduplicate(const std::vector<OHLCV>& data_list) {
    std::vector<OHLCV> result;
    std::unordered_set<std::string> seen;
    
    for (const auto& data : data_list) {
        std::string key = std::to_string(data.timestamp) + "_" + 
                         data.symbol + "_" + data.exchange;
        
        if (seen.find(key) == seen.end()) {
            seen.insert(key);
            result.push_back(data);
        }
    }
    
    return result;
}

std::vector<OHLCV> DataCleaner::fill_missing(
    const std::vector<OHLCV>& data_list,
    Timeframe timeframe,
    const std::string& method) {
    
    if (data_list.empty()) return data_list;
    
    std::vector<OHLCV> result = data_list;
    int64_t interval = timeframe_to_milliseconds(timeframe);
    
    // 简单的前向填充
    if (method == "forward") {
        // TODO: 实现前向填充逻辑
    }
    
    return result;
}

// ========== 预定义规则实现 ==========

bool PriceValidityRule::apply(OHLCV& data) const {
    return data.open > 0 && data.high > 0 && data.low > 0 && data.close > 0;
}

bool PriceJumpRule::apply(OHLCV& data) const {
    if (last_close_ > 0) {
        if (DataCleaner::detect_price_jump(data.close, last_close_, threshold_)) {
            data.quality = DataQuality::SUSPICIOUS;
        }
    }
    last_close_ = data.close;
    return true;
}

bool VolumeAnomalyRule::apply(OHLCV& data) const {
    volume_history_.push_back(data.volume);
    
    if (volume_history_.size() > HISTORY_SIZE) {
        volume_history_.erase(volume_history_.begin());
    }
    
    if (volume_history_.size() >= 10) {
        double avg_volume = std::accumulate(volume_history_.begin(), 
                                           volume_history_.end(), 0.0) / 
                           volume_history_.size();
        
        if (DataCleaner::detect_volume_anomaly(data.volume, avg_volume, threshold_)) {
            data.quality = DataQuality::SUSPICIOUS;
        }
    }
    
    return true;
}

bool OHLCRelationRule::apply(OHLCV& data) const {
    // 验证OHLC关系
    if (data.high < data.low) return false;
    if (data.high < data.open || data.high < data.close) return false;
    if (data.low > data.open || data.low > data.close) return false;
    return true;
}

} // namespace cleaners
} // namespace quant_crypto

