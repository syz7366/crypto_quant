// 兼容新版Boost的宏定义
#define BOOST_ASIO_HAS_STD_CHRONO
#define BOOST_ASIO_DISABLE_STD_CHRONO  // 禁用std::chrono
#define BOOST_ASIO_USE_TS_EXECUTOR_AS_DEFAULT
#include "websocket/binance_websocket_client.h"
#include "common/types.h"

#include<iostream>
#include<sstream>


namespace quant_crypto{
namespace websocket{

    // 构造函数和析构函数
    BinanceWebSocketClient::BinanceWebSocketClient() 
    : connected_(false), should_stop_(false) {
        // 1. 设置日志级别
        client_.set_access_channels(websocketpp::log::alevel::none);
        client_.set_error_channels(websocketpp::log::elevel::all);

        //2. 初始化ASIO
        client_.init_asio();

        //3. 设置TLS初始化处理器
        client_.set_tls_init_handler(
            std::bind(&BinanceWebSocketClient::on_tls_init, this, std::placeholders::_1)
        );

        //4. 设置消息处理器
        client_.set_message_handler(
            std::bind(&BinanceWebSocketClient::on_message, this,
                 std::placeholders::_1, std::placeholders::_2)
        );

        //5. 设置连接打开处理器
        client_.set_open_handler(
            std::bind(&BinanceWebSocketClient::on_open, this, std::placeholders::_1)
        );

        //6. 设置连接关闭处理器
        client_.set_close_handler(
            std::bind(&BinanceWebSocketClient::on_close, this, std::placeholders::_1)
        );

        //7. 设置连接失败处理器
        client_.set_fail_handler(
            std::bind(&BinanceWebSocketClient::on_fail, this, std::placeholders::_1)
        );
        std::cout << "BinanceWebSocketClient initialized" << std::endl;
    }

    BinanceWebSocketClient::~BinanceWebSocketClient() {
        stop();
    }

    // 订阅K线数据
    bool BinanceWebSocketClient::subscribe_kline(
        const std::string& symbol,
        const std::string& interval,
        KlineCallback callback){
        
        // 保存回调函数和订阅信息
        kline_callback_ = callback;
        subscribed_symbol_ = symbol;
        subscribed_interval_ = interval;

        // 构建WebSocket URL
        std::ostringstream uri_stream;
        uri_stream << "wss://fstream.binance.com/ws/" << symbol<< "@kline_" << interval;
        std::string uri = uri_stream.str();

        std::cout << "[WebSocket] 订阅K线数据: " << uri << std::endl;
        // 连接到WebSocket
        return connect(uri);
    }

    bool BinanceWebSocketClient::connect(const std::string& uri){
        try{
            // 1.创建连接
            websocketpp::lib::error_code ec;
            WebsocketClient::connection_ptr con = client_.get_connection(uri, ec);
            if(ec){
                std::cerr << "[WebSocket] ❌ 创建连接失败: " << ec.message() << std::endl;
                return false;
            }

            // 2. 保存连接句柄
            hdl_ = con->get_handle();

            // 3. 发起连接
            client_.connect(con);

            //4. 启动IO线程
            should_stop_ = false;
            io_thread_ = std::thread(&BinanceWebSocketClient::run, this);

            std::cout << "[WebSocket] ✓ 连接请求已发送" << std::endl;
            return true;
        }catch(const std::exception& e){
            std::cerr << "[WebSocket] ❌ 连接失败: " << e.what() << std::endl;
            return false;
        }
    }

    // 停止连接
    void BinanceWebSocketClient::stop(){
        if(should_stop_.load()){
            std::cout << "[WebSocket] ✅ 已停止连接" << std::endl;
            return;
        }

        std::cout << "[WebSocket] 正在停止连接..." << std::endl;
        should_stop_ = true;
        try{
            //1. 关闭WebSocket连接
            if(connected_.load()){
                websocketpp::lib::error_code ec;
                client_.close(hdl_, websocketpp::close::status::normal, "Client停止", ec);
                if(ec){
                    std::cerr << "[WebSocket] 关闭连接出错: "  << ec.message() << std::endl;
                }
            }
            //2. 停止IO服务
            client_.stop();

            //3. 等待IO线程结束
            if(io_thread_.joinable()){
                io_thread_.join();
            }

            connected_ = false;
            std::cout << "[WebSocket] ✅ 连接已停止" << std::endl;

        }catch(const std::exception& e){
            std::cerr << "[WebSocket] 停止连接出错: " << e.what() << std::endl;
        }
    }

    // 运行IO循环
    void BinanceWebSocketClient::run(){
        std::cout << "[WebSocket] IO线程启动" << std::endl;
        try {
            client_.run();
        } catch (const std::exception& e) {
            std::cerr << "[WebSocket] IO线程异常: " << e.what() << std::endl;
        }
        std::cout << "[WebSocket] IO线程退出" << std::endl;
    }

    // 消息处理函数
    void BinanceWebSocketClient::on_message(
        websocketpp::connection_hdl hdl,
        WebsocketMessagePtr msg) {
        try{
            //1. 获取消息内容
            std::string payload = msg->get_payload();

            // 2. 解析JSON
            auto json = nlohmann::json::parse(payload);

            //3. 检查消息类型（币安K线消息格式）
            if (!json.contains("e") || json["e"] != "kline") {
                return;  // 不是K线消息，忽略
            }
            //4. 提取K线数据
            auto kline_json = json["k"];

            //5. 构造OHLCV对象
            OHLCV ohlcv;
            ohlcv.timestamp = kline_json["t"];
            ohlcv.symbol = kline_json["s"];
            ohlcv.exchange = "binance";
            ohlcv.timeframe = string_to_timeframe(subscribed_interval_);
            ohlcv.open = std::stod(kline_json["o"].get<std::string>());
            ohlcv.high = std::stod(kline_json["h"].get<std::string>());
            ohlcv.low = std::stod(kline_json["l"].get<std::string>());
            ohlcv.close = std::stod(kline_json["c"].get<std::string>());
            ohlcv.volume = std::stod(kline_json["v"].get<std::string>());
            ohlcv.quote_volume = std::stod(kline_json["q"].get<std::string>());
            ohlcv.trades_count = kline_json["n"];
            ohlcv.quality = DataQuality::GOOD;

            // 6. 判断K线是否完成
            bool is_closed = kline_json["x"];       // 为什么能根据这个进行判断
            
            // 7. 调用回调函数
            if (kline_callback_) {
                kline_callback_(ohlcv);
            }

            // 8. 简单日志（仅显示已完成的K线）
            if (is_closed) {
                std::cout << "[WebSocket] ✓ K线完成: " << ohlcv.symbol 
                        << " Close=" << ohlcv.close 
                        << " Volume=" << ohlcv.volume << std::endl;
            }
        } catch(const std::exception& e){
            std::cerr << "[WebSocket] 消息处理出错: " << e.what() << std::endl;
        }
    }

    // 打开开关 的回调方法
    void BinanceWebSocketClient::on_open(websocketpp::connection_hdl hdl){
        connected_ = true;
        std::cout << "[WebSocket] ✓ 连接已建立！开始接收数据..." << std::endl;
    }

    // 关闭开关 的回调方法
    void BinanceWebSocketClient::on_close(websocketpp::connection_hdl hdl) {
        connected_ = false;
        std::cout << "[WebSocket] 连接已关闭" << std::endl;
    }

    // 连接失败回调
    void BinanceWebSocketClient::on_fail(websocketpp::connection_hdl hdl) {
        connected_ = false;
        
        // 获取失败原因
        try {
            WebsocketClient::connection_ptr con = client_.get_con_from_hdl(hdl);
            std::cerr << "[WebSocket] ❌ 连接失败: " 
                      << con->get_ec().message() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[WebSocket] ❌ 连接失败（无法获取详情）" << std::endl;
        }
    }

    // SSL/ TLS初始化
    WebsocketContext BinanceWebSocketClient::on_tls_init(
        websocketpp::connection_hdl hdl) {
        
        // 创建SSL context
        WebsocketContext ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(
            boost::asio::ssl::context::sslv23
        );
        try {
            // 设置SSL选项
            ctx->set_options(
                boost::asio::ssl::context::default_workarounds |
                boost::asio::ssl::context::no_sslv2 |
                boost::asio::ssl::context::no_sslv3 |
                boost::asio::ssl::context::single_dh_use
            );
            // 生产环境应该验证证书，这里为了简单先跳过
            ctx->set_verify_mode(boost::asio::ssl::verify_none);
            
        } catch (const std::exception& e) {
            std::cerr << "[WebSocket] SSL初始化失败: " << e.what() << std::endl;
        }
        return ctx;
    }

}
}