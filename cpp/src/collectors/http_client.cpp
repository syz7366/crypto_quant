#include "collectors/http_client.h"

namespace quant_crypto{
namespace collectors{

HttpClient::HttpClient() 
    : timeout_ms_(10000),
      proxy_port_(0){}

HttpClient::~HttpClient(){
}

void HttpClient::set_timeout(int timeout_ms){
    timeout_ms_ = timeout_ms;
}

void HttpClient::set_proxy(const std::string& proxy_host, int proxy_port) {
    proxy_host_ = proxy_host;
    proxy_port_ = proxy_port;
}

void HttpClient::add_header(const std::string& key, const std::string& value) {
    headers_[key] = value;
}

std::string HttpClient::build_query_string(
    const std::map<std::string, std::string>& params){
        if (params.empty()) return "";

        std::ostringstream oss;
        bool first = true;

        for(const auto& pair:params){
            if(!first) oss<< "&";
            oss<< pair.first << "=" << pair.second;
            first = false;
        }
    return oss.str();
}

Result<HttpResponse> HttpClient::get(const std::string& url,const std::map<std::string, std::string>& params){
    // 首先， 解析URL
    std::string host, path;

    std::cout << "[HttpClient::get] Received URL: [" << url << "]" << std::endl;
    std::cout << "[HttpClient::get] URL length: " << url.length() << std::endl;
    

    size_t proto_end = url.find("://");
    if (proto_end == std::string::npos) {
        return Result<HttpResponse>::Err(
            ErrorCode::INVALID_PARAMS,
            "URL must start with http:// or https://"
        );
    }

    size_t host_start = proto_end + 3;
    size_t path_start = url.find('/', host_start);

    
    if (path_start == std::string::npos) {
        host = url.substr(host_start);
        path = "/";
    } else {
        host = url.substr(host_start, path_start - host_start);
        path = url.substr(path_start);
    }

    std::cout << "host: " << host << ", path: " << path << std::endl;
    
    // 2. 构建查询字符串
    std::string query = build_query_string(params);
    if (!query.empty()) {
        path += "?" + query;
    }

    // 3. 创建 httplib 客户端
    httplib::SSLClient client(host);
    
    // 【关键修复1】禁用SSL证书验证（类似Python requests的verify=False）
    // 生产环境建议启用验证：client.set_ca_cert_path("/path/to/cert.pem")
    client.enable_server_certificate_verification(false);
    
    // 【关键修复2】设置代理（如果配置了代理）
    // 对应 Python 的 proxies={"http": "...", "https": "..."}
    if (!proxy_host_.empty() && proxy_port_ > 0) {
        client.set_proxy(proxy_host_.c_str(), proxy_port_);
        std::cout << "[HttpClient] ✓ 使用代理: " << proxy_host_ << ":" << proxy_port_ << std::endl;
    }

    // 4. 设置超时（类似Python的timeout=30）
    client.set_connection_timeout(timeout_ms_ / 1000, (timeout_ms_ % 1000) * 1000);
    client.set_read_timeout(timeout_ms_ / 1000, (timeout_ms_ % 1000) * 1000);
    
    // 5. 设置HTTP头（模拟浏览器/标准客户端行为）
    httplib::Headers headers;
    
    // 【关键修复3】添加标准HTTP头，避免被识别为机器人
    // 如果用户没有自定义这些头，则使用默认值
    if (headers_.find("User-Agent") == headers_.end()) {
        headers.insert({"User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36"});
    }
    if (headers_.find("Accept") == headers_.end()) {
        headers.insert({"Accept", "*/*"});
    }
    // 注意：不设置 Accept-Encoding，避免gzip压缩导致解压失败
    // httplib 在某些情况下（特别是通过代理）不能正确处理gzip压缩响应
    if (headers_.find("Connection") == headers_.end()) {
        headers.insert({"Connection", "keep-alive"});
    }
    
    // 添加用户自定义的头（会覆盖上面的默认值）
    for (const auto& h : headers_) {
        headers.insert({h.first, h.second});
    }

    // 6. 发送请求 
    std::cout << "[HttpClient] GET " << url << "?" << query << std::endl;
    
    auto res = client.Get(path, headers);
    
    // 7. 检查结果
    if (!res) {
        // 获取详细的错误信息（类似Python的异常信息）
        auto err = res.error();
        std::string error_msg;
        
        switch (err) {
            case httplib::Error::Connection:
                error_msg = "Connection failed (SSL握手失败或网络不可达)";
                break;
            case httplib::Error::BindIPAddress:
                error_msg = "Bind IP address failed";
                break;
            case httplib::Error::Read:
                error_msg = "Read error (连接中断)";
                break;
            case httplib::Error::Write:
                error_msg = "Write error";
                break;
            case httplib::Error::ExceedRedirectCount:
                error_msg = "Exceed redirect count";
                break;
            case httplib::Error::Canceled:
                error_msg = "Request canceled";
                break;
            case httplib::Error::SSLConnection:
                error_msg = "SSL connection error (证书验证失败)";
                break;
            case httplib::Error::SSLLoadingCerts:
                error_msg = "SSL loading certs error";
                break;
            case httplib::Error::SSLServerVerification:
                error_msg = "SSL server verification error";
                break;
            case httplib::Error::UnsupportedMultipartBoundaryChars:
                error_msg = "Unsupported multipart boundary chars";
                break;
            default:
                error_msg = "Unknown error";
                break;
        }
        
        std::cerr << "[HttpClient] ❌ Request failed: " << error_msg << std::endl;
        return Result<HttpResponse>::Err(
            ErrorCode::NETWORK_ERROR,
            "Failed to connect to " + host + ": " + error_msg
        );
    }

    // 8. 构建响应（保存响应头）
    HttpResponse response;
    response.status_code = res->status;
    response.body = res->body;
    
    // 保存响应头（类似Python的response.headers）
    for (const auto& header : res->headers) {
        response.headers[header.first] = header.second;
    }

    std::cout << "[HttpClient] ✓ Status: " << response.status_code 
              << ", Body size: " << response.body.size() << " bytes" << std::endl;

    // 注意：不在这里判断状态码，让调用者决定如何处理非200状态
    // 因为有些API的204/201也是成功的
    return Result<HttpResponse>::Ok(response);
}

Result<HttpResponse> HttpClient::post(const std::string& url,const std::string& body,const std::string& content_type){
    // 解析URL
    std::string host, path;
    size_t proto_end = url.find("://");
    if (proto_end == std::string::npos) {
        return Result<HttpResponse>::Err(
            ErrorCode::INVALID_PARAMS,
            "URL must start with http:// or https://"
        );
    }

    size_t host_start = proto_end + 3;
    size_t path_start = url.find('/', host_start);

    
    if (path_start == std::string::npos) {
        host = url.substr(host_start);
        path = "/";
    } else {
        host = url.substr(host_start, path_start - host_start);
        path = url.substr(path_start);
    }

    std::cout << "host: " << host << ", path: " << path << std::endl;
    
    // 创建 httplib 客户端
    httplib::SSLClient client(host);
    
    // 禁用SSL证书验证（与GET方法保持一致）
    client.enable_server_certificate_verification(false);
    
    // 设置代理（如果配置了代理）
    if (!proxy_host_.empty() && proxy_port_ > 0) {
        client.set_proxy(proxy_host_.c_str(), proxy_port_);
        std::cout << "[HttpClient] ✓ 使用代理: " << proxy_host_ << ":" << proxy_port_ << std::endl;
    }

    // 设置超时
    client.set_connection_timeout(timeout_ms_ / 1000, (timeout_ms_ % 1000) * 1000);
    client.set_read_timeout(timeout_ms_ / 1000, (timeout_ms_ % 1000) * 1000);
    
    // 5. 设置HTTP头（与GET方法保持一致）
    httplib::Headers headers;
    
    // 添加标准HTTP头
    if (headers_.find("User-Agent") == headers_.end()) {
        headers.insert({"User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36"});
    }
    if (headers_.find("Accept") == headers_.end()) {
        headers.insert({"Accept", "*/*"});
    }
    // 不设置 Accept-Encoding（避免gzip压缩问题）
    if (headers_.find("Connection") == headers_.end()) {
        headers.insert({"Connection", "keep-alive"});
    }
    
    // 添加用户自定义的头
    for (const auto& h : headers_) {
        headers.insert({h.first, h.second});
    }

    // 6. 发送请求
    std::cout << "[HttpClient] POST " << url << " with body: " << body << std::endl;
    
    auto res = client.Post(path, headers, body, content_type);
    
    // 7. 检查结果
    if (!res) {
        std::cerr << "[HttpClient] Request failed: network error" << std::endl;
        return Result<HttpResponse>::Err(
            ErrorCode::NETWORK_ERROR,
            "Failed to connect to " + host
        );
    }

    // 8. 构建响应
    HttpResponse response;
    response.status_code = res->status;
    response.body = res->body;

    std::cout << "[HttpClient] Status: " << response.status_code << std::endl;

    if (response.status_code != 200) {
        return Result<HttpResponse>::Err(
            ErrorCode::HTTP_ERROR,
            "HTTP " + std::to_string(response.status_code)
        );
    }
    return Result<HttpResponse>::Ok(response);
}

}


}