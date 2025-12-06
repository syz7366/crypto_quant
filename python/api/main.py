"""
FastAPI主应用入口
"""

import sys
from pathlib import Path

# 添加项目根目录到Python路径
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from fastapi.staticfiles import StaticFiles
import uvicorn

from python.config.settings import get_settings
from python.api.routes import klines, orderbook, ticker, health, backtest

# 创建FastAPI应用
app = FastAPI(
    title="加密货币量化交易数据层API",
    description="提供标准化的加密货币市场数据接口",
    version="1.0.0",
    docs_url="/docs",
    redoc_url="/redoc",
)

# 配置CORS
settings = get_settings()
app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.cors_origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


# 全局异常处理
@app.exception_handler(Exception)
async def global_exception_handler(request, exc):
    return JSONResponse(
        status_code=500,
        content={
            "error": "Internal Server Error",
            "message": str(exc),
            "path": str(request.url),
        },
    )


# 注册路由
app.include_router(health.router, prefix="/api/v1", tags=["健康检查"])
app.include_router(klines.router, prefix="/api/v1", tags=["K线数据"])
app.include_router(orderbook.router, prefix="/api/v1", tags=["订单簿"])
app.include_router(ticker.router, prefix="/api/v1", tags=["行情"])
app.include_router(backtest.router, prefix="/api/v1", tags=["回测"])

# 挂载静态文件目录（用于可视化页面）
static_path = Path(__file__).parent / "static"
static_path.mkdir(exist_ok=True)
app.mount("/static", StaticFiles(directory=str(static_path)), name="static")


@app.on_event("startup")
async def startup_event():
    """应用启动事件"""
    print("=" * 60)
    print("🚀 加密货币量化交易数据层API启动中...")
    print(f"📝 文档地址: http://{settings.host}:{settings.port}/docs")
    print(f"🔧 环境: {settings.environment}")
    print("=" * 60)


@app.on_event("shutdown")
async def shutdown_event():
    """应用关闭事件"""
    print("👋 应用正在关闭...")


if __name__ == "__main__":
    uvicorn.run(
        "main:app",
        host=settings.host,
        port=settings.port,
        reload=settings.debug,
        log_level="info",
    )

