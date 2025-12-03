#include "websocket/websocket_factory.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>

using namespace quant_crypto;

// 全局变量用于信号处理
std::atomic<bool> g_running{true};

void signal_handler(int signal) {
    std::cout << "\n收到中断信号，正在关闭..." << std::endl;
    g_running = false;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "WebSocket 测试程序 (Boost.Beast)" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // 设置信号处理
    std::signal(SIGINT, signal_handler);
    
    try {
        // 1. 创建IO上下文     这里IO上下文的意义是什么
        net::io_context ioc;
        
        // 2. 创建SSL上下文
        ssl::context ctx{ssl::context::tlsv12_client};
        
        // 加载系统根证书
        ctx.set_default_verify_paths();
        ctx.set_verify_mode(ssl::verify_peer);
        
        // 3. 使用工厂创建客户端
        auto client = ws::WebSocketFactory::create("binance", ioc, ctx);
        
        if (!client) {
            std::cerr << "❌ 创建客户端失败" << std::endl;
            return 1;
        }
        
        // 4. 记录收到的K线数量
        int kline_count = 0;
        
        // 5. 订阅K线数据
        client->subscribe_kline("BTCUSDT", "1s", [&kline_count](const OHLCV& ohlcv) {
            kline_count++;
            
            // 每收到10条打印一次统计
            if (kline_count % 10 == 0) {
                std::cout << "\n📊 已收到 " << kline_count << " 条K线数据" << std::endl;
            }
        });
        
        // 6. 在后台线程运行IO循环 —————— 这里循环IO的意义是什么
        std::thread io_thread([&ioc]() {
            std::cout << "[Main] IO线程启动" << std::endl;
            ioc.run();
            std::cout << "[Main] IO线程结束" << std::endl;
        });
        
        // 7. 等待连接建立
        std::cout << "等待连接建立..." << std::endl;
        int wait_count = 0;
        while (!client->is_connected() && wait_count < 100 && g_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            // 如果去掉这些代码会发生什么呢
            wait_count++;
        }
        
        if (!client->is_connected()) {
            std::cerr << "❌ 连接超时" << std::endl;
        } else {
            std::cout << "✅ 连接已建立！" << std::endl;
        }
        
        // 8. 主线程等待用户中断
        std::cout << "\n💡 按 Ctrl+C 停止...\n" << std::endl;
        
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // 9. 断开连接
        std::cout << "\n正在断开连接..." << std::endl;
        client->disconnect();
        
        // 10. 停止IO上下文
        ioc.stop();
        
        // 11. 等待IO线程结束
        if (io_thread.joinable()) {
            io_thread.join();
        }
        
        // 12. 打印统计
        std::cout << "\n========================================" << std::endl;
        std::cout << "测试结束" << std::endl;
        std::cout << "总共收到 " << kline_count << " 条K线数据" << std::endl;
        std::cout << "========================================" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}