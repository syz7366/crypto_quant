#pragma once

#include <string>
#include <json.hpp>

namespace quant_crypto {
namespace config {

/**
 * @struct BinanceConfig
 * @brief 币安配置结构（从JSON读取）
 */
struct BinanceConfig {
    std::string base_url;
    std::string proxy_host;
    int proxy_port;
    bool proxy_enabled;
    int timeout_ms;
    std::string data_dir;
};

/**
 * @class ConfigManager
 * @brief 配置管理类（单例模式）
 */
class ConfigManager {
public:
    /**
     * @brief 加载配置文件
     * @param config_file 配置文件路径（如 "config/binance.json"）
     * @return 是否加载成功
     */
    static bool load(const std::string& config_file);
    
    /**
     * @brief 获取币安配置
     * @return BinanceConfig对象
     */
    static BinanceConfig get_binance_config();
    
private:
    static nlohmann::json config_data_;  // 存储配置数据
    static bool loaded_;                  // 是否已加载
};

} // namespace config
} // namespace quant_crypto