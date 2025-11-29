#include "collectors/binance_collector.h"
#include "common/market_data.h"
#include "common/types.h"
#include <iostream>
#include <json.hpp>
#include <string>
using json = nlohmann::json;

namespace quant_crypto{
namespace collectors{
    BinanceCollector::BinanceCollector(const BinanceConfig& config)
        : config_(config),
        base_url_(config.base_url)
        {
            // http_client_ = HttpClient();  这里是不需要的
            // 这里如果代理为空，本身就没有配置，那么这里设置会不会导致什么差错出现
            if(!config_.proxy_host.empty() && config_.proxy_port > 0){
                http_client_.set_proxy(config_.proxy_host, config_.proxy_port);
            }
            http_client_.set_timeout(config_.timeout_ms);
            std::cout << "[BinanceCollector] 初始化完成" << std::endl;
            std::cout << "[BinanceCollector] Base URL: " << base_url_ << std::endl;
        
        }
    BinanceCollector::~BinanceCollector(){
        std::cout << "[BinanceCollector] 销毁" << std::endl;
    }
    Result<std::vector<common::Kline>> BinanceCollector::get_klines(
        const std::string& symbol, 
        const std::string& interval, 
        int limit)
    {
            // 1.构造完成的URL
            std::string url = base_url_ + "/api/v3/klines";
            
            // 2.构建查询参数
            std::map<std::string, std::string> params;
            params["symbol"] = symbol;
            params["interval"] = interval;
            params["limit"] = std::to_string(limit);

            // 3. 调试输出
            std::cout << "[BinanceCollector] 请求URL: " << url << std::endl;
            std::cout << "[BinanceCollector] 参数: symbol=" << symbol 
            << ", interval=" << interval 
            << ", limit=" << limit << std::endl;
            auto result = http_client_.get(url, params);
            if(!result.success){
                std::cerr << "[BinanceCollector] 请求失败: " << result.error_message << std::endl;
                return Result<std::vector<common::Kline>>::Err(result.error_code, "获取K线数据失败"+result.error_message);
            }
            auto response = result.data;
            std::cout << "[BinanceCollector] HTTP请求成功，状态码: " 
            << response.status_code << std::endl;
            std::cout << "[BinanceCollector] 响应体大小: " 
            << response.body.size() << " bytes" << std::endl;

            // 4. 解析JSON响应
            std::vector<common::Kline> klines;
            try{
                auto json = nlohmann::json::parse(response.body);
                for(const auto& kline : json){
                    common::Kline k;
                    k.open_time = kline[0];
                    k.open = std::stod(kline[1].get<std::string>());
                    k.high = std::stod(kline[2].get<std::string>());
                    k.low = std::stod(kline[3].get<std::string>());
                    k.close = std::stod(kline[4].get<std::string>());
                    k.volume = std::stod(kline[5].get<std::string>());

                    // 将结果添加到vector中
                    klines.push_back(k);
                }
                std::cout << "[BinanceCollector] 成功解析 " << klines.size() << " 条K线数据" << std::endl;
                return Result<std::vector<common::Kline>>::Ok(klines);
            }catch(const std::exception& e){
                std::cerr << "[BinanceCollector] JSON解析失败: " << e.what() << std::endl;
                return Result<std::vector<common::Kline>>::Err(ErrorCode::PARSE_ERROR, "解析K线数据失败"+std::string(e.what()));
            }
    }

    Result<common::Ticker> BinanceCollector::get_ticker(const std::string& symbol){

        // 1. 构建完成的URL
        std::string url = base_url_ + "/api/v3/ticker/24hr";
        
        // 2. 构建查询参数
        std::map<std::string, std::string> params;
        params["symbol"] = symbol;

        // 3. 调试输出
        std::cout << "[BinanceCollector] 请求URL: " << url << std::endl;
        std::cout << "[BinanceCollector] 参数: symbol=" << symbol << std::endl;

        // 4. 发送请求
        auto result = http_client_.get(url, params);

        // 5. 检查HTTP请求是否成功
        if(!result.success){
            std::cerr << "[BinanceCollector] 请求失败: " << result.error_message << std::endl;
            return Result<common::Ticker>::Err(result.error_code, "获取24小时价格统计失败"+result.error_message);
        }
        auto response = result.data;

        // 6. 解析JSON响应
        try{
            common::Ticker ticker;
            auto json = nlohmann::json::parse(response.body);    
            ticker.symbol = json["symbol"].get<std::string>();
            ticker.last_price = std::stod(json["lastPrice"].get<std::string>());
            ticker.high_price = std::stod(json["highPrice"].get<std::string>());
            ticker.low_price = std::stod(json["lowPrice"].get<std::string>());
            ticker.volume = std::stod(json["volume"].get<std::string>());
            ticker.price_change_percent = std::stod(json["priceChangePercent"].get<std::string>());
            return Result<common::Ticker>::Ok(ticker);
        }catch(const std::exception& e){
            std::cerr << "[BinanceCollector] JSON解析失败: " << e.what() << std::endl;
            return Result<common::Ticker>::Err(ErrorCode::PARSE_ERROR, "解析24小时价格统计失败"+std::string(e.what()));
        };
    }

    Result<common::OrderBook> BinanceCollector::get_orderbook(const std::string& symbol, int limit) {
        // 1. 构建URL
        std::string url = base_url_ + "/api/v3/depth";
        
        // 2. 构建查询参数
        std::map<std::string, std::string> params;
        params["symbol"] = symbol;
        params["limit"] = std::to_string(limit);
        
        std::cout << "[BinanceCollector] 请求OrderBook数据..." << std::endl;
        std::cout << "[BinanceCollector] Symbol: " << symbol << ", Limit: " << limit << std::endl;
        
        // 3. 发送HTTP请求
        auto http_result = http_client_.get(url, params);
        
        if (!http_result.success) {
            std::cerr << "[BinanceCollector] HTTP请求失败: " 
                      << http_result.error_message << std::endl;
            return Result<common::OrderBook>::Err(
                http_result.error_code,
                "获取OrderBook数据失败: " + http_result.error_message
            );
        }
        
        auto response = http_result.data;
        std::cout << "[BinanceCollector] HTTP请求成功，状态码: " 
                  << response.status_code << std::endl;
        
        // 4. 解析JSON（⚠️ 注意：这里结构比较复杂）
        try {
            auto json_data = nlohmann::json::parse(response.body);
            
            common::OrderBook orderbook;
            orderbook.symbol = symbol;
            
            // ⚠️ 解析bids数组（买单）- 每个元素是 [price, quantity]
            for (const auto& bid : json_data["bids"]) {
                common::PriceLevel level;
                level.price = std::stod(bid[0].get<std::string>());
                level.quantity = std::stod(bid[1].get<std::string>());
                orderbook.bids.push_back(level);
            }
            
            // ⚠️ 解析asks数组（卖单）- 结构相同
            for (const auto& ask : json_data["asks"]) {
                common::PriceLevel level;
                level.price = std::stod(ask[0].get<std::string>());
                level.quantity = std::stod(ask[1].get<std::string>());
                orderbook.asks.push_back(level);
            }
            
            std::cout << "[BinanceCollector] 成功解析OrderBook: " 
                      << orderbook.bids.size() << " bids, " 
                      << orderbook.asks.size() << " asks" << std::endl;
            
            return Result<common::OrderBook>::Ok(orderbook);
            
        } catch (const std::exception& e) {
            std::cerr << "[BinanceCollector] JSON解析失败: " << e.what() << std::endl;
            return Result<common::OrderBook>::Err(
                ErrorCode::PARSE_ERROR,
                "解析OrderBook数据失败: " + std::string(e.what())
            );
        }
    }


}
}   
