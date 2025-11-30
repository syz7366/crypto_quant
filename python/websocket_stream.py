#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
币安WebSocket实时数据流
"""

import websocket
import json
import time
from datetime import datetime

class BinanceWebSocketStream:
    """币安WebSocket实时数据流客户端"""
    
    def __init__(self):
        self.ws = None
        self.message_count = 0
        
    def on_message(self, ws, message):
        """接收到消息时的回调"""
        try:
            data = json.loads(message)
            
            # 检查是否是K线数据
            if data.get('e') == 'kline':
                kline = data['k']
                
                # 提取关键信息
                symbol = kline['s']
                interval = kline['i']
                open_price = float(kline['o'])
                high_price = float(kline['h'])
                low_price = float(kline['l'])
                close_price = float(kline['c'])
                volume = float(kline['v'])
                is_closed = kline['x']  # K线是否已完成
                
                # 格式化时间
                timestamp = kline['t']
                dt = datetime.fromtimestamp(timestamp / 1000)
                time_str = dt.strftime('%Y-%m-%d %H:%M:%S')
                
                # 消息计数
                self.message_count += 1
                
                # 打印数据
                status = "✅ 完成" if is_closed else "📊 更新中"
                print(f"\n[{self.message_count}] {status} {symbol} {interval} - {time_str}")
                print(f"  OHLC: {open_price:.2f} / {high_price:.2f} / {low_price:.2f} / {close_price:.2f}")
                print(f"  成交量: {volume:.4f}")
                
                # K线完成时特别提示
                if is_closed:
                    print(f"  🎯 K线已完成！可以保存数据")
                    
        except Exception as e:
            print(f"❌ 解析消息失败: {e}")
    
    def on_error(self, ws, error):
        """连接错误时的回调"""
        print(f"❌ WebSocket错误: {error}")
    
    def on_close(self, ws, close_status_code, close_msg):
        """连接关闭时的回调"""
        print(f"\n🔌 WebSocket连接已关闭")
        print(f"状态码: {close_status_code}, 消息: {close_msg}")
    
    def on_open(self, ws):
        """连接打开时的回调"""
        print("✅ WebSocket连接已建立！")
        print("🚀 开始接收实时数据...\n")
    
    def subscribe_kline(self, symbol, interval):
        """
        订阅K线数据流
        
        Args:
            symbol: 交易对（如 'btcusdt'，小写）
            interval: 时间周期（如 '1m', '5m', '1h'）
        """
        # 构建WebSocket URL
        url = f"wss://stream.binance.com:9443/ws/{symbol}@kline_{interval}"
        
        print("=" * 60)
        print("币安WebSocket实时数据流")
        print("=" * 60)
        print(f"订阅: {symbol.upper()} {interval}")
        print(f"URL: {url}")
        print("=" * 60)
        print()
        
        # 创建WebSocket连接
        self.ws = websocket.WebSocketApp(
            url,
            on_open=self.on_open,
            on_message=self.on_message,
            on_error=self.on_error,
            on_close=self.on_close
        )
        
        # 运行（会阻塞）- 添加代理支持
        self.ws.run_forever(
            http_proxy_host='127.0.0.1',
            http_proxy_port=7897,
            proxy_type='http'
        )


def main():
    """主函数"""
    import threading
    
    # 创建客户端
    client = BinanceWebSocketStream()
    
    # 30秒后自动停止
    def stop_after_delay():
        time.sleep(30)
        print("\n\n⏰ 30秒测试完成，自动停止...")
        if client.ws:
            client.ws.close()
    
    timer = threading.Thread(target=stop_after_delay, daemon=True)
    timer.start()
    
    # 订阅BTCUSDT 1分钟K线
    try:
        client.subscribe_kline('btcusdt', '1m')
    except KeyboardInterrupt:
        print("\n\n👋 用户中断，程序退出")
    except Exception as e:
        print(f"\n❌ 程序异常: {e}")


if __name__ == '__main__':
    main()

