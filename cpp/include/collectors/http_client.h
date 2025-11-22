#pragma once

namespace quant_crypto{
namespace collectors{

struct HttpResponse{
    int status_code;
    std::string body;
    std::map<std::string, std::string> headers;
};

class HttpClient{
    HttpClient();
    ~HttpClient();

    /**
     * @brief 发送GET请求
     * @param timeout_ms 请求超时时间(毫秒)
     */
    void set_timeout(int timeout_ms);

    /**
     * @brief 设置代理
     * @param proxy_host 代理地址
     * @param proxy_port 代理端口
     */
    void set_proxy(const std::string& proxy_host, int proxy_port);

    /**
     * @brief 添加请求头
     * @param key 头名称
     * @param value 头值
     */
    void add_header(const std::string& key, const std::string& value);

    /**
     * @brief 发送 GET 请求
     * @param url 完整URL（包含 https://）
     * @param params 请求体
     * @return HTTP 响应结果
     */
    Result<HttpResponse> get(
        const std::string& url, 
        const std::map<std::string, std::string>& params = {}
    );

    /**
     * @brief 发送 POST 请求
     * @param url 完整URL
     * @param body 请求体
     * @param content_type 内容类型
     * @return HTTP 响应结果
     */
     Result<HttpResponse> post(
        const std::string& url,
        const std::string& body,
        const std::string& content_type = "application/json"
    );

private:
    int timeout_ms_;
    std::string proxy_host_;
    int proxy_port_;
    std::map<std::string, std::string> headers_;

    // 构建 URL 查询字符串
    std::string build_query_string(const std::map<std::string, std::string>& params);
}




}
}

