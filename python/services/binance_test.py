

# 临时测试用， 验证数据采集链路
# Binance API 测试脚本
import ccxt 
import json
from datetime import datetime, timedelta

def test_ccxt_api():

    exchange = ccxt.binance({
        'enableRateLimit': True,
        'timeout': 1500,
        'proxies': {
        "http": "http://127.0.0.1:7897",
        "https": "http://127.0.0.1:7897",
    }
    })

    markets = exchange.load_markets()
    print(list(markets.keys())[:10])


def test_direct_api():
    """直接调用Binance原始API（不用ccxt）"""
    
    import requests
    
    # Binance K线API端点
    url = "https://api.binance.com/api/v3/klines"
    
    params = {
        'symbol': 'BTCUSDT',  
        'interval': '1h',
        'limit': 100
    }
    
    print(f"\n请求URL: {url}")
    print(f"参数: {params}")
    
    try:
        response = requests.get(
            url, 
            params=params, 
            timeout=30  # 超时时间（秒）
        )
        response.raise_for_status()
        
        data = response.json()
        
        print(f"\n✅ 成功! 状态码: {response.status_code}")
        for item in data:
            print(item)
        return data
        
    except Exception as e:
        return None


if __name__ == "__main__":

    # 这里是通过ccxt 自己主动去拉取的
    # test_ccxt_api()

    # 测试3: 直接调用原始API
    raw_data = test_direct_api()
    print(raw_data)