#pragma once

#include <string>
#include <vector>
#include "common/types.h"

namespace quant_crypto {
namespace storage {
/**
 * @class KlineStorage
 * @brief K线数据CSV存储类
 */
class KlineStorage {
public:
    /**
     * @brief 构造函数
     * @param data_dir 数据存储目录（如 "data/"）
     */
    KlineStorage(const std::string& data_dir);
    
    /**
     * @brief 保存K线数据到CSV文件
     * @param symbol 交易对（如 "BTCUSDT"）
     * @param interval 时间周期（如 "1h"）
     * @param ohlcv_list OHLCV数据列表
     * @return 是否成功
     */
    bool save_ohlcv(
        const std::string& symbol,
        const std::string& interval,
        const std::vector<OHLCV>& ohlcv_list
    );
    
private:
    std::string data_dir_;  // 数据目录
    
    // 辅助方法：生成文件名
    std::string generate_filename(
        const std::string& symbol, 
        const std::string& interval
    ) const;
};
    
} // namespace storage
} // namespace quant_crypto






