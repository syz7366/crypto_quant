#!/usr/bin/env python3
"""
数据库初始化脚本
"""

import sys
from pathlib import Path

# 添加项目根目录到Python路径
sys.path.insert(0, str(Path(__file__).parent.parent))

import asyncio
import asyncpg
from loguru import logger

from python.config.settings import get_settings


async def create_database():
    """创建数据库（如果不存在）"""
    settings = get_settings()

    # 连接到postgres数据库
    try:
        conn = await asyncpg.connect(
            host=settings.db_host,
            port=settings.db_port,
            user=settings.db_user,
            password=settings.db_password,
            database="postgres",
        )

        # 检查数据库是否存在
        db_exists = await conn.fetchval(
            "SELECT 1 FROM pg_database WHERE datname = $1", settings.db_name
        )

        if not db_exists:
            logger.info(f"创建数据库: {settings.db_name}")
            await conn.execute(f'CREATE DATABASE "{settings.db_name}"')
        else:
            logger.info(f"数据库已存在: {settings.db_name}")

        await conn.close()

    except Exception as e:
        logger.error(f"创建数据库失败: {e}")
        raise


async def setup_timescaledb():
    """设置TimescaleDB扩展"""
    settings = get_settings()

    try:
        conn = await asyncpg.connect(
            host=settings.db_host,
            port=settings.db_port,
            user=settings.db_user,
            password=settings.db_password,
            database=settings.db_name,
        )

        # 创建TimescaleDB扩展
        logger.info("启用TimescaleDB扩展...")
        await conn.execute("CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE")

        await conn.close()
        logger.info("✓ TimescaleDB扩展已启用")

    except Exception as e:
        logger.warning(f"启用TimescaleDB扩展失败: {e}")
        logger.warning("将使用普通PostgreSQL模式")


async def create_tables():
    """创建数据表"""
    settings = get_settings()

    try:
        conn = await asyncpg.connect(
            host=settings.db_host,
            port=settings.db_port,
            user=settings.db_user,
            password=settings.db_password,
            database=settings.db_name,
        )

        # K线数据表
        logger.info("创建K线数据表...")
        await conn.execute(
            """
            CREATE TABLE IF NOT EXISTS klines (
                timestamp BIGINT NOT NULL,
                symbol VARCHAR(50) NOT NULL,
                exchange VARCHAR(50) NOT NULL,
                timeframe VARCHAR(10) NOT NULL,
                open DOUBLE PRECISION NOT NULL,
                high DOUBLE PRECISION NOT NULL,
                low DOUBLE PRECISION NOT NULL,
                close DOUBLE PRECISION NOT NULL,
                volume DOUBLE PRECISION NOT NULL,
                quote_volume DOUBLE PRECISION NOT NULL,
                trades_count BIGINT DEFAULT 0,
                quality SMALLINT DEFAULT 0,
                created_at TIMESTAMPTZ DEFAULT NOW(),
                PRIMARY KEY (timestamp, symbol, exchange, timeframe)
            )
        """
        )

        # 尝试创建超表（TimescaleDB）
        try:
            await conn.execute(
                """
                SELECT create_hypertable('klines', 'timestamp',
                    chunk_time_interval => 86400000,
                    if_not_exists => TRUE
                )
            """
            )
            logger.info("✓ K线超表创建成功")
        except Exception:
            logger.warning("超表创建失败，使用普通表")

        # 创建索引
        await conn.execute(
            """
            CREATE INDEX IF NOT EXISTS idx_klines_symbol_exchange_timeframe
            ON klines (symbol, exchange, timeframe, timestamp DESC)
        """
        )

        # Tick数据表
        logger.info("创建Tick数据表...")
        await conn.execute(
            """
            CREATE TABLE IF NOT EXISTS ticks (
                timestamp BIGINT NOT NULL,
                symbol VARCHAR(50) NOT NULL,
                exchange VARCHAR(50) NOT NULL,
                price DOUBLE PRECISION NOT NULL,
                volume DOUBLE PRECISION NOT NULL,
                side VARCHAR(10),
                trade_id VARCHAR(100),
                created_at TIMESTAMPTZ DEFAULT NOW(),
                PRIMARY KEY (timestamp, symbol, exchange, trade_id)
            )
        """
        )

        # 订单簿数据表
        logger.info("创建订单簿数据表...")
        await conn.execute(
            """
            CREATE TABLE IF NOT EXISTS orderbooks (
                timestamp BIGINT NOT NULL,
                symbol VARCHAR(50) NOT NULL,
                exchange VARCHAR(50) NOT NULL,
                bids JSONB NOT NULL,
                asks JSONB NOT NULL,
                sequence BIGINT DEFAULT 0,
                created_at TIMESTAMPTZ DEFAULT NOW(),
                PRIMARY KEY (timestamp, symbol, exchange)
            )
        """
        )

        # Ticker数据表
        logger.info("创建Ticker数据表...")
        await conn.execute(
            """
            CREATE TABLE IF NOT EXISTS tickers (
                timestamp BIGINT NOT NULL,
                symbol VARCHAR(50) NOT NULL,
                exchange VARCHAR(50) NOT NULL,
                last DOUBLE PRECISION NOT NULL,
                bid DOUBLE PRECISION,
                ask DOUBLE PRECISION,
                high_24h DOUBLE PRECISION,
                low_24h DOUBLE PRECISION,
                volume_24h DOUBLE PRECISION,
                quote_volume_24h DOUBLE PRECISION,
                change_24h DOUBLE PRECISION,
                created_at TIMESTAMPTZ DEFAULT NOW(),
                PRIMARY KEY (timestamp, symbol, exchange)
            )
        """
        )

        # 交易对信息表
        logger.info("创建交易对信息表...")
        await conn.execute(
            """
            CREATE TABLE IF NOT EXISTS symbols (
                id SERIAL PRIMARY KEY,
                symbol VARCHAR(50) NOT NULL,
                exchange VARCHAR(50) NOT NULL,
                base_asset VARCHAR(20) NOT NULL,
                quote_asset VARCHAR(20) NOT NULL,
                active BOOLEAN DEFAULT TRUE,
                min_price DOUBLE PRECISION,
                max_price DOUBLE PRECISION,
                price_precision INTEGER DEFAULT 8,
                volume_precision INTEGER DEFAULT 8,
                metadata JSONB,
                created_at TIMESTAMPTZ DEFAULT NOW(),
                updated_at TIMESTAMPTZ DEFAULT NOW(),
                UNIQUE(symbol, exchange)
            )
        """
        )

        await conn.close()
        logger.info("✓ 所有表创建完成")

    except Exception as e:
        logger.error(f"创建表失败: {e}")
        raise


async def main():
    """主函数"""
    logger.info("=" * 60)
    logger.info("🗄️  开始初始化数据库...")
    logger.info("=" * 60)

    try:
        # 1. 创建数据库
        await create_database()

        # 2. 设置TimescaleDB
        await setup_timescaledb()

        # 3. 创建表
        await create_tables()

        logger.info("")
        logger.info("=" * 60)
        logger.info("✅ 数据库初始化完成！")
        logger.info("=" * 60)

    except Exception as e:
        logger.error(f"数据库初始化失败: {e}")
        sys.exit(1)


if __name__ == "__main__":
    asyncio.run(main())

