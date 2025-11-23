# 🌐 代理配置说明

## 📋 代理配置已添加

我已经在 C++ 测试程序中添加了代理支持，对应你的 Python 代码：

### Python 版本 (binance_test.py)
```python
proxies = {
    "http": "http://127.0.0.1:7897",
    "https": "http://127.0.0.1:7897",
}
```

### C++ 版本 (test_binance_minimal.cpp)
```cpp
// 文件开头的配置
constexpr bool USE_PROXY = true;  // 是否使用代理
constexpr const char* PROXY_HOST = "127.0.0.1";
constexpr int PROXY_PORT = 7897;
```

---

## 🔧 如何配置代理

### 方法1：启用代理（默认已启用）

打开 `cpp/src/collectors/test_binance_minimal.cpp`，找到文件开头的配置部分：

```cpp
// 是否使用代理（类似Python的proxies参数）
constexpr bool USE_PROXY = true;  // ✅ 启用代理

// 代理服务器地址和端口
constexpr const char* PROXY_HOST = "127.0.0.1";
constexpr int PROXY_PORT = 7897;
```

### 方法2：禁用代理（直连）

如果你的网络可以直连币安API，修改为：

```cpp
constexpr bool USE_PROXY = false;  // ❌ 禁用代理
```

### 方法3：修改代理地址

如果你使用不同的代理端口：

```cpp
constexpr const char* PROXY_HOST = "127.0.0.1";
constexpr int PROXY_PORT = 1087;  // 改为你的代理端口
```

---

## 🚀 重新编译和测试

修改配置后，需要重新编译：

```bash
cd /Users/songhao/Desktop/quant_crypto/build

# 重新编译（很快，只编译这一个文件）
make test_binance_minimal

# 运行测试
./bin/test_binance_minimal
```

---

## 📊 预期输出（使用代理）

```
╔════════════════════════════════════════════════════╗
║     币安API最小化测试程序                          ║
║     Binance API Minimal Test                      ║
╚════════════════════════════════════════════════════╝

当前配置：
  代理: 启用 (127.0.0.1:7897)
  SSL验证: 禁用（测试模式）

========================================
测试：访问币安服务器时间API
========================================
✓ 已配置代理: 127.0.0.1:7897
请求URL: https://api.binance.com/api/v3/time
✅ 状态码: 200
响应: {"serverTime":1701234567890}
服务器时间戳: 1701234567890

========================================
测试：访问币安K线API
========================================
✓ 已配置代理: 127.0.0.1:7897
请求URL: https://api.binance.com/api/v3/klines?symbol=BTCUSDT&interval=1h&limit=5
发送请求中...

✅ 连接成功！
...
```

---

## ❌ 常见错误

### 错误1：代理连接失败（错误代码14）

```
❌ 请求失败！
错误代码: 14
错误类型: Proxy Connection (代理连接失败)
可能原因: 无法连接到代理服务器 127.0.0.1:7897
```

**解决方案**：
1. 检查代理软件是否正在运行
2. 检查端口号是否正确（7897）
3. 尝试用curl测试代理：
   ```bash
   curl -x http://127.0.0.1:7897 https://api.binance.com/api/v3/time
   ```

### 错误2：直连失败（Connection）

如果禁用代理后仍然失败：

```
错误类型: Connection (连接失败/SSL握手失败)
```

**可能原因**：
- 你的网络环境需要代理才能访问国际网站
- 防火墙阻止了连接
- DNS解析问题

**解决方案**：
- 启用代理：`USE_PROXY = true`
- 或者使用VPN

---

## 🎯 最佳实践

### 1. 开发环境
```cpp
// 开发环境通常需要代理
constexpr bool USE_PROXY = true;
constexpr const char* PROXY_HOST = "127.0.0.1";
constexpr int PROXY_PORT = 7897;
```

### 2. 生产环境
```cpp
// 生产环境通常直连
constexpr bool USE_PROXY = false;
```

### 3. 灵活配置
未来可以考虑从配置文件或环境变量读取：
```cpp
// 从环境变量读取（将来的改进）
const char* proxy_host = std::getenv("PROXY_HOST");
const char* proxy_port_str = std::getenv("PROXY_PORT");
```

---

## 📝 快速检查清单

测试前确认：

- [ ] 代理软件正在运行（如果启用代理）
- [ ] 端口号配置正确（默认7897）
- [ ] 已重新编译程序
- [ ] Python版本可以正常访问（作为参考）

---

## 🔄 对比Python和C++的代理设置

| 特性 | Python | C++ |
|------|--------|-----|
| 配置方式 | 字典参数 | 编译时常量 |
| 灵活性 | 运行时修改 | 需重新编译 |
| 性能 | 慢 | 快 |
| 代码位置 | requests参数 | client.set_proxy() |

**注意**：未来如果需要运行时配置，可以修改为从配置文件读取。

---

**现在可以重新编译测试了！** 🚀

