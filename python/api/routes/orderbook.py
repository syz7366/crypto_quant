"""
订单簿路由
"""

from typing import List
from fastapi import APIRouter, Query, HTTPException
from pydantic import BaseModel

router = APIRouter()


class OrderBookLevel(BaseModel):
    """订单簿档位"""

    price: float
    volume: float


class OrderBookResponse(BaseModel):
    """订单簿响应模型"""

    timestamp: int
    symbol: str
    exchange: str
    bids: List[OrderBookLevel]
    asks: List[OrderBookLevel]
    sequence: int


@router.get("/orderbook", response_model=OrderBookResponse)
async def get_orderbook(
    symbol: str = Query(..., description="交易对，如 BTC/USDT"),
    exchange: str = Query(..., description="交易所，如 binance"),
    depth: int = Query(20, ge=5, le=100, description="深度档位数"),
):
    """
    获取订单簿数据
    
    参数:
        - symbol: 交易对
        - exchange: 交易所
        - depth: 深度档位数
        
    返回:
        订单簿快照
    """
    # TODO: 实现实际的数据查询逻辑
    raise HTTPException(
        status_code=501, detail="此接口尚未实现，请在开发过程中实现数据查询逻辑"
    )

