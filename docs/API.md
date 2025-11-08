# API 文档

## 基础信息

- **Base URL**: `http://localhost:8000/api/v1`
- **认证方式**: JWT Bearer Token（待实现）
- **数据格式**: JSON

## 健康检查

### GET /health

检查服务健康状态

**请求示例:**
```bash
curl http://localhost:8000/api/v1/health
```

**响应示例:**
```json
{
  "status": "healthy",
  "timestamp": 1699999999999,
  "version": "1.0.0",
  "components": {
    "api": "ok",
    "database": "ok",
    "cache": "ok",
    "cpp_core": "ok"
  }
}
```

---

## K线数据

### GET /klines

获取K线（OHLCV）数据

**查询参数:**

| 参数 | 类型 | 必填 | 说明 | 示例 |
|-----|------|------|------|------|
| symbol | string | 是 | 交易对 | BTC/USDT |
| exchange | string | 是 | 交易所 | binance |
| timeframe | string | 是 | 时间周期 | 1m, 5m, 1h, 1d |
| start | integer | 否 | 开始时间戳（毫秒） | 1609459200000 |
| end | integer | 否 | 结束时间戳（毫秒） | 1609545600000 |
| limit | integer | 否 | 返回数量限制 (1-1000) | 100 |

**请求示例:**
```bash
curl "http://localhost:8000/api/v1/klines?symbol=BTC/USDT&exchange=binance&timeframe=1h&limit=100"
```

**响应示例:**
```json
[
  {
    "timestamp": 1609459200000,
    "symbol": "BTC/USDT",
    "exchange": "binance",
    "timeframe": "1h",
    "open": 28750.50,
    "high": 28850.00,
    "low": 28700.00,
    "close": 28800.00,
    "volume": 123.456,
    "quote_volume": 3554280.50,
    "trades_count": 5432
  }
]
```

---

## 订单簿

### GET /orderbook

获取当前订单簿快照

**查询参数:**

| 参数 | 类型 | 必填 | 说明 | 示例 |
|-----|------|------|------|------|
| symbol | string | 是 | 交易对 | BTC/USDT |
| exchange | string | 是 | 交易所 | binance |
| depth | integer | 否 | 深度档位数 (5-100) | 20 |

**请求示例:**
```bash
curl "http://localhost:8000/api/v1/orderbook?symbol=BTC/USDT&exchange=binance&depth=20"
```

**响应示例:**
```json
{
  "timestamp": 1699999999999,
  "symbol": "BTC/USDT",
  "exchange": "binance",
  "bids": [
    {"price": 28799.50, "volume": 1.234},
    {"price": 28799.00, "volume": 2.456}
  ],
  "asks": [
    {"price": 28800.00, "volume": 1.123},
    {"price": 28800.50, "volume": 2.345}
  ],
  "sequence": 123456789
}
```

---

## Ticker（行情）

### GET /ticker

获取24小时行情统计

**查询参数:**

| 参数 | 类型 | 必填 | 说明 | 示例 |
|-----|------|------|------|------|
| symbol | string | 是 | 交易对 | BTC/USDT |
| exchange | string | 是 | 交易所 | binance |

**请求示例:**
```bash
curl "http://localhost:8000/api/v1/ticker?symbol=BTC/USDT&exchange=binance"
```

**响应示例:**
```json
{
  "timestamp": 1699999999999,
  "symbol": "BTC/USDT",
  "exchange": "binance",
  "last": 28800.00,
  "bid": 28799.50,
  "ask": 28800.50,
  "high_24h": 29200.00,
  "low_24h": 28500.00,
  "volume_24h": 12345.678,
  "quote_volume_24h": 355428050.50,
  "change_24h": 1.25
}
```

---

## 错误响应

所有接口在出错时返回统一格式：

```json
{
  "error": "Error Type",
  "message": "详细错误信息",
  "path": "/api/v1/klines"
}
```

**HTTP状态码:**

- `200` - 成功
- `400` - 请求参数错误
- `401` - 未授权
- `404` - 资源不存在
- `429` - 请求过于频繁
- `500` - 服务器内部错误
- `501` - 接口未实现

---

## WebSocket API（待实现）

### 连接地址

```
ws://localhost:8000/ws/v1
```

### 订阅K线
```json
{
  "action": "subscribe",
  "channel": "kline",
  "symbol": "BTC/USDT",
  "exchange": "binance",
  "timeframe": "1m"
}
```

### 订阅订单簿
```json
{
  "action": "subscribe",
  "channel": "orderbook",
  "symbol": "BTC/USDT",
  "exchange": "binance",
  "depth": 20
}
```

---

## 速率限制

- 默认: 60 请求/分钟
- Burst: 10 请求/秒

超过限制将返回 `429 Too Many Requests`

---

## 认证（待实现）

使用JWT Bearer Token认证：

```bash
curl -H "Authorization: Bearer YOUR_TOKEN" \
  http://localhost:8000/api/v1/klines?symbol=BTC/USDT&exchange=binance&timeframe=1h
```

