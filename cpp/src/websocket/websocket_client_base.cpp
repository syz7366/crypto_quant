#include "websocket/websocket_client_base.h"
#include <iostream>

namespace quant_crypto{
namespace ws{

    // 构造函数和析构函数
    WebSocketClientBase::WebSocketClientBase(
        net::io_context& ioc,
        ssl::context& ctx, 
        const std::string& exchange_name)
    :resolver_(net::make_strand(ioc))    // 为什么这里 可以确保回调在同一个线程中执行，是通过什么机制实现的
    ,ws_(net::make_strand(ioc),ctx)
    ,exchange_name_(exchange_name)
    ,connected_(false)
    ,should_stop_(false)
    {
        std::cout << "[" << exchange_name_ << "] WebSocket客户端初始化" << std::endl;
    }

    WebSocketClientBase::~WebSocketClientBase(){
        disconnect();
        std::cout << "[" << exchange_name_ << "] WebSocket客户端销毁" << std::endl;
    }

// ============ 公共接口 ==============
void WebSocketClientBase::connect(
    const std::string& host,
    const std::string& port,
    const std::string& path
){
    host_ = host;
    path_ = path;

    std::cout << "[" << exchange_name_ << "] 正在连接: wss://" 
              << host << ":" << port << path << std::endl;

    // 开始异步解析DNS    这里这个resolver_ 对象具体是干什么用的
    resolver_.async_resolve(
        host,
        port,
        beast::bind_front_handler(
            &WebSocketClientBase::on_resolve,
            shared_from_this()
        )
    );
}

void WebSocketClientBase::disconnect(){
    if(!connected_) return;

    should_stop_ = true;

    // 异步关闭WebSocket 连接
    ws_.async_close(      // 我对这里这个方法的具体认识不太到位，不知道这个方法具体是如何作用的
        websocket::close_code::normal,
        beast::bind_front_handler(           // 为什么这里的绑定比std::bind 更加高效
            &WebSocketClientBase::on_close,
            shared_from_this()
        )
    );
}

void  WebSocketClientBase::run(){
    // 这个方法由外部 io_context.run 驱动
    // 这里暂时先留空

}

void WebSocketClientBase::send_message(const std::string& message){
    if(!connected_){
        std::cerr << "[" <<exchange_name_<< "] 未连接，无法发送消息" << std::endl;
        return;
    }

    std::cout<< "[" << exchange_name_ <<"] 发送: " << message << std::endl;

    //  ws_ 对象的核心作用是什么，一般如何使用，这里似乎是将写操作绑定到 一个对象上执行 
    ws_.async_write(
        net::buffer(message),
        beast::bind_front_handler(
            &WebSocketClientBase::on_write,
            shared_from_this())
    );
}

void WebSocketClientBase::do_read(){
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &WebSocketClientBase::on_read,
            shared_from_this()   // 这个方法具体是在做什么，意义是什么
        )
    );
}

// =============== 异步回调实现 =================
// 这个方法的作用是什么，具体意义是什么
void WebSocketClientBase::on_resolve(
    beast::error_code ec,
    tcp::resolver::results_type results
){
    if(ec) {
        std::cerr <<"[" << exchange_name_ << "] DNS解析失败: " << ec.message() << std::endl;
        return;
    }

    std::cout << "[" << exchange_name_ << "] DNS解析成功" << std::endl;
    
    // 设置TCP连接超时
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // 开始异步连接
    beast::get_lowest_layer(ws_).async_connect(
        results,
        beast::bind_front_handler(
            &WebSocketClientBase::on_connect,
            shared_from_this()
        )
    );
}

// 这个方法的意义是什么，具体有什么作用
void WebSocketClientBase::on_connect(
    beast::error_code ec,
    tcp::resolver::results_type::endpoint_type ep
){
    if(ec) {
        std::cerr << "[" << exchange_name_ << "] TCP连接失败: " << ec.message() << std::endl;
        return;
    }

    std::cout << "[" << exchange_name_ << "] TCP连接成功: " 
              << ep.address().to_string() << ":" << ep.port() << std::endl;
    
    // 设置SSL超时
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // 设置SNI（Server Name Indication）  设置这个的意义是什么
    if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host_.c_str())) {
        ec = beast::error_code(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category());
        std::cerr << "[" << exchange_name_ << "] SNI设置失败: " << ec.message() << std::endl;
        return;
    }

    // 开始SSL 握手
    ws_.next_layer().async_handshake(
        ssl::stream_base::client,
        beast::bind_front_handler(
            &WebSocketClientBase::on_ssl_handshake,
            shared_from_this()
        )
    );
}

void WebSocketClientBase::on_ssl_handshake(beast::error_code ec){
    if(ec){
        std::cerr << "[" << exchange_name_ << "] SSL握手失败: " << ec.message() << std::endl;
        return;
    }
    std::cout << "[" << exchange_name_ << "] SSL握手成功" << std::endl;

    // 关闭超时 （WebSocket 有自己的ping/pong机制）  这里这个语句做了什么工作
    beast::get_lowest_layer(ws_).expires_never();

    // 设置WebSocket选项
    ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
    ws_.set_option(websocket::stream_base::decorator(
        [this](websocket::request_type& req) {
            req.set(beast::http::field::user_agent, "QuantCrypto/1.0");
            req.set(beast::http::field::host, host_);
        }
    ));

    // 开始WebSocket 握手
    ws_.async_handshake(host_, path_, beast::bind_front_handler(
        &WebSocketClientBase::on_handshake,
        shared_from_this()
    ));
}

void WebSocketClientBase::on_handshake(beast::error_code ec){
    if(ec){
        std::cerr << "[" << exchange_name_ << "] WebSocket握手失败: " << ec.message() << std::endl;
        return;
    }

    std::cout << "[" << exchange_name_ << "] WebSocket握手成功" << std::endl;
    connected_ = true;
    // 开始读取WebSocket数据
    do_read();
}

void WebSocketClientBase::on_write(beast::error_code ec, std::size_t bytes_transferred){
    if (ec) {
        std::cerr << "[" << exchange_name_ << "] 发送失败: " << ec.message() << std::endl;
        return;
    }
    std::cout << "[" << exchange_name_ << "] 发送成功: " << bytes_transferred << " bytes" << std::endl;
}

void WebSocketClientBase::on_read(
    beast::error_code ec,
    std::size_t bytes_transferred
){
    if (ec) {
        if (ec == websocket::error::closed) {
            std::cout << "[" << exchange_name_ << "] 连接已关闭" << std::endl;
        } else {
            std::cerr << "[" << exchange_name_ << "] 读取失败: " << ec.message() << std::endl;
        }
        connected_ = false;
        return;
    }
    
    // 获取消息内容
    std::string message = beast::buffers_to_string(buffer_.data());
    buffer_.consume(buffer_.size());  // 这个方法的作用是什么，具体意义是什么
    
    // 调用子类的解析方法
    parse_message(message);
    
    // 如果没有停止，继续读取
    if (!should_stop_) {
        do_read();  // 为什么可以这么继续调用
    }
}

void WebSocketClientBase::on_close(beast::error_code ec){
    if (ec) {
        std::cerr << "[" << exchange_name_ << "] 关闭失败: " << ec.message() << std::endl;
    } else {
        std::cout << "[" << exchange_name_ << "] 连接已正常关闭" << std::endl;
    }
    connected_ = false;
}


}
}






