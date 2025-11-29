#include "config/config_manager.h"
#include <fstream>
#include <iostream>

namespace quant_crypto {
namespace config {

// 静态成员变量定义
nlohmann::json ConfigManager::config_data_;
bool ConfigManager::loaded_ = false;

/**
 * @brief 加载配置文件
 * @param config_file 配置文件路径
 * @return 是否加载成功
 * 
 * 依据用户要求：读取config/binance.json文件并解析
 */
bool ConfigManager::load(const std::string& config_file){
    std::cout<< "[ConfigManager] 正在加载配置文件: " << config_file << std::endl;

    // 1. 打开文件
    std::ifstream file(config_file);
    if(!file.is_open()){
        std::cerr<< "[ConfigManager] 无法打开配置文件: " << config_file << std::endl;
        return false;
    }

    // 2. 解析JSON
    try{
        file >> config_data_;
        loaded_ = true; 
        std::cout<< "[ConfigManager] 配置文件加载成功" << std::endl;
        return true;
    }catch(const std::exception& e){
        std::cerr<< "[ConfigManager] 解析配置文件失败: " << e.what() << std::endl;
        loaded_ = false;
        return false;
    }
}


// ========================================================================
// get_binance_config() 方法：从JSON中提取币安配置
// ========================================================================
/**
 * @brief 获取币安配置
 * @return BinanceConfig结构体
 * 
 * 从config_data_中提取各个字段
 */

BinanceConfig ConfigManager::get_binance_config() {
    if (!loaded_) {
        std::cerr << "[ConfigManager] ⚠️  配置未加载，返回默认配置" << std::endl;
        // 返回默认配置
        return BinanceConfig{
            "https://api.binance.com",
            "127.0.0.1",
            7897,
            false,
            30000,
            "data"
        };
    }
    // 从JSON中提取配置
    BinanceConfig config;
    try {
        // 基础配置
        config.base_url = config_data_.value("base_url", "https://api.binance.com");
        config.timeout_ms = config_data_.value("timeout_ms", 30000);
        config.data_dir = config_data_.value("data_dir", "data");
        
        // 代理配置（嵌套对象）
        if (config_data_.contains("proxy")) {
            auto proxy = config_data_["proxy"];
            config.proxy_enabled = proxy.value("enabled", false);
            config.proxy_host = proxy.value("host", "127.0.0.1");
            config.proxy_port = proxy.value("port", 7897);
        } else {
            config.proxy_enabled = false;
            config.proxy_host = "127.0.0.1";
            config.proxy_port = 7897;
        }
        // 打印加载的配置（调试用）
        std::cout << "[ConfigManager] 配置详情:" << std::endl;
        std::cout << "  - Base URL: " << config.base_url << std::endl;
        std::cout << "  - Proxy: " << (config.proxy_enabled ? "启用" : "禁用") << std::endl;
        if (config.proxy_enabled) {
            std::cout << "    Host: " << config.proxy_host << ":" << config.proxy_port << std::endl;
        }
        std::cout << "  - Timeout: " << config.timeout_ms << "ms" << std::endl;
        std::cout << "  - Data Dir: " << config.data_dir << std::endl;
        
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "[ConfigManager] ❌ 解析配置失败: " << e.what() << std::endl;
    }
    return config;
}


} // namespace config
} // namespace quant_crypto