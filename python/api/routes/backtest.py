"""
回测API路由
提供策略回测和性能分析接口
"""

from fastapi import APIRouter, HTTPException
from pydantic import BaseModel, Field
from typing import List, Optional

# 从父包导入已经加载的 C++ 模块（避免重复导入）
try:
    import quant_crypto_core as qcc
except ImportError:
    # 如果直接导入失败，尝试从 python 包导入
    import sys
    from pathlib import Path
    sys.path.insert(0, str(Path(__file__).parent.parent.parent))
    import quant_crypto_core as qcc

router = APIRouter()


# ========== 请求模型 ==========
class BacktestRequest(BaseModel):
    """回测请求参数"""
    symbol: str = Field(default="BTCUSDT", description="交易对")
    interval: str = Field(default="1h", description="K线周期")
    limit: int = Field(default=500, description="K线数量", ge=100, le=5000)
    
    # 策略参数
    strategy_name: str = Field(default="ma_cross", description="策略名称")
    fast_period: int = Field(default=10, description="快线周期", ge=2, le=100)
    slow_period: int = Field(default=30, description="慢线周期", ge=5, le=200)
    position_size: float = Field(default=1.0, description="仓位大小", ge=0.1, le=1.0)
    
    # 回测参数
    initial_capital: float = Field(default=10000.0, description="初始资金", gt=0)
    commission_rate: float = Field(default=0.001, description="手续费率", ge=0, le=0.01)
    slippage_rate: float = Field(default=0.0005, description="滑点率", ge=0, le=0.01)


class TradeInfo(BaseModel):
    """交易信息"""
    timestamp: int
    signal: str
    price: float
    quantity: float
    pnl: float


class PerformanceMetrics(BaseModel):
    """性能指标"""
    # 收益指标
    annualized_return: float
    cumulative_return: float
    
    # 风险指标
    max_drawdown: float
    sharpe_ratio: float
    sortino_ratio: float
    calmar_ratio: float
    volatility: float
    downside_deviation: float
    
    # 交易指标
    profit_loss_ratio: float
    max_consecutive_wins: int
    max_consecutive_losses: int
    avg_holding_period: float
    trade_frequency_per_year: float


class BacktestResponse(BaseModel):
    """回测响应"""
    success: bool
    message: str
    
    # 回测结果
    initial_capital: float
    final_equity: float
    total_return: float
    total_trades: int
    winning_trades: int
    losing_trades: int
    
    # 曲线数据
    equity_curve: List[float]
    drawdown_curve: List[float]
    timestamps: List[int]
    
    # 交易记录
    trades: List[TradeInfo]
    
    # 性能指标
    metrics: PerformanceMetrics


# ========== API端点 ==========
@router.post("/backtest/run", response_model=BacktestResponse)
async def run_backtest(request: BacktestRequest):
    """
    运行策略回测
    
    流程：
    1. 获取历史K线数据
    2. 创建策略实例
    3. 运行回测引擎
    4. 进行性能分析
    5. 返回结果
    """
    
    try:
        # ========== 1. 获取历史数据 ==========
        binance_config = qcc.BinanceConfig()
        binance_config.base_url = "https://api.binance.com"
        binance_config.proxy_enabled = False
        binance_config.timeout_ms = 10000
        
        collector = qcc.BinanceCollector(binance_config)
        klines_result = collector.get_klines(request.symbol, request.interval, request.limit)
        
        if not klines_result.success or len(klines_result.data) == 0:
            raise HTTPException(
                status_code=400,
                detail=f"获取K线数据失败: {klines_result.error_message}"
            )
        
        # ========== 2. 创建策略 ==========
        strategy_config = qcc.MACrossConfig()
        strategy_config.fast_period = request.fast_period
        strategy_config.slow_period = request.slow_period
        strategy_config.position_size = request.position_size
        
        strategy = qcc.MACrossStrategy(strategy_config)
        
        # ========== 3. 运行回测 ==========
        backtest_config = qcc.BacktestConfig()
        backtest_config.initial_capital = request.initial_capital
        backtest_config.commission_rate = request.commission_rate
        backtest_config.slippage_rate = request.slippage_rate
        
        engine = qcc.BacktestEngine(backtest_config)
        engine.set_strategy(strategy)
        engine.set_data(klines_result.data)
        engine.run()
        
        backtest_result = engine.get_result()
        
        # ========== 4. 性能分析 ==========
        analyzer = qcc.PerformanceAnalyzer()
        metrics = analyzer.analyze(
            backtest_result.equity_curve,
            backtest_result.timestamps,
            backtest_result.trades,
            request.initial_capital
        )
        
        # ========== 5. 构造响应 ==========
        trades_info = []
        for trade in backtest_result.trades:
            signal_str = "BUY" if trade.signal == qcc.Signal.BUY else "SELL"
            trades_info.append(TradeInfo(
                timestamp=trade.timestamp,
                signal=signal_str,
                price=trade.price,
                quantity=trade.quantity,
                pnl=trade.pnl
            ))
        
        performance_metrics = PerformanceMetrics(
            annualized_return=metrics.annualized_return,
            cumulative_return=metrics.cumulative_return,
            max_drawdown=metrics.max_drawdown,
            sharpe_ratio=metrics.sharpe_ratio,
            sortino_ratio=metrics.sortino_ratio,
            calmar_ratio=metrics.calmar_ratio,
            volatility=metrics.volatility,
            downside_deviation=metrics.downside_deviation,
            profit_loss_ratio=metrics.profit_loss_ratio,
            max_consecutive_wins=metrics.max_consecutive_wins,
            max_consecutive_losses=metrics.max_consecutive_losses,
            avg_holding_period=metrics.avg_holding_period,
            trade_frequency_per_year=metrics.trade_frequency_per_year
        )
        
        return BacktestResponse(
            success=True,
            message="回测完成",
            initial_capital=backtest_result.initial_capital,
            final_equity=backtest_result.final_equity,
            total_return=backtest_result.total_return,
            total_trades=backtest_result.total_trades,
            winning_trades=backtest_result.winning_trades,
            losing_trades=backtest_result.losing_trades,
            equity_curve=list(backtest_result.equity_curve),
            drawdown_curve=list(metrics.drawdown_curve),
            timestamps=list(backtest_result.timestamps),
            trades=trades_info,
            metrics=performance_metrics
        )
        
    except Exception as e:
        raise HTTPException(
            status_code=500,
            detail=f"回测执行失败: {str(e)}"
        )


@router.get("/backtest/strategies")
async def list_strategies():
    """
    获取可用的策略列表
    """
    return {
        "strategies": [
            {
                "name": "ma_cross",
                "display_name": "均线交叉策略",
                "description": "基于快慢均线交叉的趋势跟踪策略",
                "parameters": {
                    "fast_period": {"type": "int", "default": 10, "min": 2, "max": 100},
                    "slow_period": {"type": "int", "default": 30, "min": 5, "max": 200},
                    "position_size": {"type": "float", "default": 1.0, "min": 0.1, "max": 1.0}
                }
            }
        ]
    }

