"""
行情Ticker路由
"""

from fastapi import APIRouter, Query, HTTPException
from pydantic import BaseModel

router = APIRouter()


class TickerResponse(BaseModel):
    """Ticker响应模型"""

    timestamp: int
    symbol: str
    exchange: str
    last: float
    bid: float
    ask: float
    high_24h: float
    low_24h: float
    volume_24h: float
    quote_volume_24h: float
    change_24h: float


@router.get("/ticker", response_model=TickerResponse)
async def get_ticker(
    symbol: str = Query(..., description="交易对，如 BTC/USDT"),
    exchange: str = Query(..., description="交易所，如 binance"),
):
    """
    获取Ticker数据
    
    参数:
        - symbol: 交易对
        - exchange: 交易所
        
    返回:
        24小时行情数据
    """
    # TODO: 实现实际的数据查询逻辑
    raise HTTPException(
        status_code=501, detail="此接口尚未实现，请在开发过程中实现数据查询逻辑"
    )

