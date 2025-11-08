"""
健康检查路由
"""

from fastapi import APIRouter
from pydantic import BaseModel
import time

router = APIRouter()


class HealthResponse(BaseModel):
    status: str
    timestamp: int
    version: str
    components: dict


@router.get("/health", response_model=HealthResponse)
async def health_check():
    """
    健康检查接口
    
    返回:
        - status: 服务状态
        - timestamp: 当前时间戳
        - version: API版本
        - components: 各组件状态
    """
    return HealthResponse(
        status="healthy",
        timestamp=int(time.time() * 1000),
        version="1.0.0",
        components={
            "api": "ok",
            "database": "ok",  # TODO: 实际检查数据库连接
            "cache": "ok",  # TODO: 实际检查Redis连接
            "cpp_core": "ok",  # TODO: 实际检查C++模块
        },
    )


@router.get("/ping")
async def ping():
    """简单的ping接口"""
    return {"message": "pong", "timestamp": int(time.time() * 1000)}

