# 开发指南

## 项目结构说明

### C++ 部分 (cpp/)

C++ 负责性能关键部分的实现：

1. **数据采集** (`collectors/`)
   - 实现WebSocket连接管理
   - 实现REST API请求
   - 处理实时数据流
   - 连接池和重连机制

2. **数据标准化** (`normalizers/`)
   - 统一不同交易所的数据格式
   - 时间戳标准化
   - 价格和数量精度处理

3. **数据清洗** (`cleaners/`)
   - 异常数据检测
   - 数据完整性校验
   - 缺失值处理
   - 去重逻辑

4. **存储接口** (`storage/`)
   - 数据库连接管理
   - 批量写入优化
   - 缓存管理

### Python 部分 (python/)

Python 负责业务逻辑和服务层：

1. **API服务** (`api/`)
   - FastAPI RESTful接口
   - WebSocket推送
   - 认证授权
   - 请求验证

2. **业务服务** (`services/`)
   - 数据查询服务
   - 数据聚合服务
   - 回测数据服务

3. **存储层** (`storage/`)
   - ORM模型定义
   - 数据库操作封装
   - Redis缓存操作

4. **配置管理** (`config/`)
   - 配置文件解析
   - 环境变量管理
   - 运行时配置

5. **监控** (`monitoring/`)
   - Prometheus指标收集
   - 性能监控
   - 告警逻辑

6. **任务调度** (`tasks/`)
   - 定时任务
   - 数据回填任务
   - 清理任务

## 开发流程

### 1. C++ 模块开发

#### 步骤1: 定义接口
在 `cpp/include/` 中定义头文件

```cpp
// cpp/include/collectors/binance_collector.h
#pragma once
#include "collectors/base_collector.h"

class BinanceCollector : public BaseCollector {
public:
    bool initialize() override;
    bool start() override;
    // ... 其他接口实现
};
```

#### 步骤2: 实现功能
在 `cpp/src/` 中实现源文件

```cpp
// cpp/src/collectors/binance_collector.cpp
#include "collectors/binance_collector.h"

bool BinanceCollector::initialize() {
    // 实现初始化逻辑
}
```

#### 步骤3: 添加Python绑定
在 `cpp/bindings/bindings.cpp` 中添加

```cpp
py::class_<BinanceCollector, BaseCollector>(m, "BinanceCollector")
    .def(py::init<>())
    .def("initialize", &BinanceCollector::initialize);
```

#### 步骤4: 编译测试
```bash
./scripts/build.sh
python3 -c "import python.quant_crypto_core as core; print(dir(core))"
```

### 2. Python 模块开发

#### 步骤1: 创建服务类
```python
# python/services/kline_service.py
class KlineService:
    def __init__(self):
        self.core = quant_crypto_core
        
    async def get_klines(self, symbol, timeframe):
        # 调用C++模块或数据库
        pass
```

#### 步骤2: 创建API路由
```python
# python/api/routes/klines.py
@router.get("/klines")
async def get_klines(symbol: str):
    service = KlineService()
    return await service.get_klines(symbol)
```

#### 步骤3: 添加测试
```python
# tests/unit/test_kline_service.py
def test_get_klines():
    service = KlineService()
    result = await service.get_klines("BTC/USDT", "1m")
    assert result is not None
```

## 编码规范

### C++ 规范

1. **命名规则**
   - 类名: `PascalCase`
   - 函数名: `snake_case`
   - 变量名: `snake_case`
   - 常量: `UPPER_SNAKE_CASE`
   - 成员变量: `trailing_underscore_`

2. **代码风格**
   - 使用4空格缩进
   - 每行不超过100字符
   - 头文件使用 `#pragma once`
   - 使用智能指针管理内存

3. **注释**
   - 使用Doxygen风格注释
   - 每个公开接口必须有文档

### Python 规范

1. **遵循PEP 8**
   - 使用4空格缩进
   - 每行不超过100字符
   - 使用 `black` 格式化代码

2. **类型提示**
   - 所有函数必须有类型提示
   - 使用 `mypy` 检查类型

3. **文档**
   - 使用Google风格docstring
   - 每个公开API必须有文档

## 性能优化建议

### C++ 性能优化

1. **避免频繁内存分配**
   - 使用对象池
   - 预分配缓冲区
   - 使用 `std::move` 移动语义

2. **并发优化**
   - 使用线程池
   - 避免锁竞争
   - 使用无锁数据结构

3. **编译优化**
   - Release模式: `-O3 -march=native`
   - 使用PGO（Profile-Guided Optimization）
   - 启用LTO（Link-Time Optimization）

### Python 性能优化

1. **异步编程**
   - 使用 `async/await`
   - 批量操作避免循环

2. **调用C++模块**
   - 批量传递数据
   - 避免频繁Python-C++边界跨越

3. **缓存策略**
   - 使用Redis缓存热数据
   - 实现LRU缓存

## 调试技巧

### C++ 调试

```bash
# 使用gdb调试
gdb --args python3 -c "import python.quant_crypto_core"

# 使用valgrind检查内存泄漏
valgrind --leak-check=full python3 test_script.py
```

### Python 调试

```python
# 使用pdb调试
import pdb; pdb.set_trace()

# 使用loguru日志
from loguru import logger
logger.debug("调试信息")
```

## 测试

### 运行所有测试
```bash
pytest tests/ -v --cov=python
```

### 运行特定测试
```bash
pytest tests/unit/test_kline_service.py -v
```

### 性能测试
```bash
pytest tests/performance/ --benchmark-only
```

## 部署

### Docker部署
```bash
# 启动基础设施
docker-compose up -d

# 构建应用
./scripts/build.sh

# 运行服务
python -m python.api.main
```

### 生产环境
```bash
# 使用gunicorn + uvicorn
gunicorn python.api.main:app \
    --workers 4 \
    --worker-class uvicorn.workers.UvicornWorker \
    --bind 0.0.0.0:8000
```

## 常见问题

### Q: C++模块编译失败
A: 检查依赖是否安装完整，查看CMake错误日志

### Q: Python导入C++模块失败
A: 检查虚拟环境，确保模块在正确路径

### Q: 数据库连接失败
A: 检查docker-compose服务是否启动，检查配置文件

## 更多资源

- [CMake文档](https://cmake.org/documentation/)
- [pybind11文档](https://pybind11.readthedocs.io/)
- [FastAPI文档](https://fastapi.tiangolo.com/)
- [TimescaleDB文档](https://docs.timescale.com/)

