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

Result<HttpResponse> HttpClient::get(const std::string& url,
const std::map<std::string, std::string>& params){
    // 首先， 解析URL
    std::string host, path;
    size_t proto_end = url.find("://");
    if (proto_end != std::string::npos) {
        return Result<HttpResponse>::Err(
            ErrorCode::INVALID_PARAMS,
            "URL must start with http:// or https://"
        );
    }

    size_t host_start = proto_end + 3;
    size_t path_start = url.find('/', host_start);

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
    httplib::SSLClient client(host);  //HTTPS 用 SSLClient

    // 4. 设置超时
    client.set_connection_timeout(timeout_ms_ / 1000, (timeout_ms_ % 1000) * 1000);
    client.set_read_timeout(timeout_ms_ / 1000, (timeout_ms_ % 1000) * 1000);
    
    // 5. 设置自定义头
    httplib::Headers headers;
    for (const auto& h : headers_) {
        headers.insert({h.first, h.second});
    }

    // 6. 发送请求 
    std::cout << "[HttpClient] GET " << url << "?" << query << std::endl;
    
    auto res = client.Get(path, headers);

    // 7. 检查结果
    if (!res) {
        std::cerr << "[HttpClient] Request failed: network error" << std::endl;
        return Result<HttpResponse>::Err(
            ErrorCode::NETWORK_ERROR,
            "Failed to connect to " + host
        );
    }

    // 构建响应
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

Result<HttpResponse> HttpClient::post(const std::string& url,
const std::string& body,
const std::string& content_type){
    // 解析URL
    std::string host, path;
    size_t proto_end = url.find("://");
    if (proto_end != std::string::npos) {
        return Result<HttpResponse>::Err(
            ErrorCode::INVALID_PARAMS,
            "URL must start with http:// or https://"
        );
    }

    size_t host_start = proto_end + 3;
    size_t path_start = url.find('/', host_start);

    size_t path_start = url.find('/', host_start);
    
    if (path_start == std::string::npos) {
        host = url.substr(host_start);
        path = "/";
    } else {
        host = url.substr(host_start, path_start - host_start);
        path = url.substr(path_start);
    }

    std::cout << "host: " << host << ", path: " << path << std::endl;
    
    //  创建 httplib 客户端
    httplib::SSLClient client(host);  //HTTPS 用 SSLClient

    // 4. 设置超时
    client.set_connection_timeout(timeout_ms_ / 1000, (timeout_ms_ % 1000) * 1000);
    client.set_read_timeout(timeout_ms_ / 1000, (timeout_ms_ % 1000) * 1000);
    
    // 5. 设置自定义头
    httplib::Headers headers;
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
}