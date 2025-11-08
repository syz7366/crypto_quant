# 加密货币量化交易系统 - 数据层

## 项目简介

一个高性能的加密货币量化交易数据层系统，采用 C++ 和 Python 混合开发：
- **C++**: 处理性能关键部分（数据采集、标准化、清洗）
- **Python**: 处理业务逻辑（API服务、配置管理、监控）

## 架构设计

```
┌─────────────────────────────────────────────────────┐
│           Python API Layer (FastAPI)                │
│  - RESTful API                                      │
│  - WebSocket 推送                                   │
│  - 认证授权                                         │
└─────────────────┬───────────────────────────────────┘
                  │
┌─────────────────▼───────────────────────────────────┐
│         Python Business Layer                       │
│  - 任务调度                                         │
│  - 配置管理                                         │
│  - 监控日志                                         │
└─────────────────┬───────────────────────────────────┘
                  │ (pybind11)
┌─────────────────▼───────────────────────────────────┐
│         C++ Core Layer (高性能)                     │
│  - 数据采集器 (WebSocket/REST)                      │
│  - 数据标准化                                       │
│  - 数据清洗                                         │
│  - 内存数据结构                                     │
└─────────────────┬───────────────────────────────────┘
                  │
┌─────────────────▼───────────────────────────────────┐
│         Storage Layer                               │
│  - TimescaleDB (时序数据)                           │
│  - Redis (缓存)                                     │
│  - PostgreSQL (元数据)                              │
└─────────────────────────────────────────────────────┘
```

## 技术栈

### C++ 部分
- **C++17/20**
- **CMake** - 构建系统
- **pybind11** - Python绑定
- **libcurl** - HTTP客户端
- **websocketpp** - WebSocket客户端
- **nlohmann/json** - JSON解析
- **spdlog** - 日志库
- **Boost.Asio** - 异步IO

### Python 部分
- **Python 3.10+**
- **FastAPI** - API框架
- **asyncio** - 异步编程
- **pydantic** - 数据验证
- **SQLAlchemy** - ORM
- **redis-py** - Redis客户端
- **prometheus-client** - 监控
- **loguru** - 日志

## 目录结构

```
.
├── cpp/                        # C++ 核心代码
│   ├── include/               # 头文件
│   │   ├── collectors/        # 数据采集器
│   │   ├── normalizers/       # 数据标准化
│   │   ├── cleaners/          # 数据清洗
│   │   ├── storage/           # 存储接口
│   │   └── utils/             # 工具类
│   ├── src/                   # 源文件
│   ├── bindings/              # Python绑定
│   ├── tests/                 # C++ 测试
│   └── CMakeLists.txt         # CMake配置
│
├── python/                     # Python 代码
│   ├── api/                   # FastAPI 服务
│   │   ├── routes/            # 路由
│   │   ├── schemas/           # 数据模型
│   │   └── deps.py            # 依赖注入
│   ├── services/              # 业务服务
│   ├── storage/               # 存储层封装
│   ├── config/                # 配置管理
│   ├── monitoring/            # 监控
│   ├── tasks/                 # 定时任务
│   └── utils/                 # 工具函数
│
├── config/                     # 配置文件
│   ├── exchanges.yaml         # 交易所配置
│   ├── settings.yaml          # 系统配置
│   └── logging.yaml           # 日志配置
│
├── tests/                      # 集成测试
│   ├── unit/                  # 单元测试
│   └── integration/           # 集成测试
│
├── scripts/                    # 脚本
│   ├── build.sh               # 构建脚本
│   ├── setup_db.py            # 数据库初始化
│   └── backfill_data.py       # 数据回填
│
├── docker/                     # Docker配置
│   ├── Dockerfile.cpp         # C++ 构建镜像
│   ├── Dockerfile.python      # Python 运行镜像
│   └── docker-compose.yml     # 编排配置
│
├── docs/                       # 文档
│   ├── api.md                 # API文档
│   └── architecture.md        # 架构文档
│
├── CMakeLists.txt             # 根CMake
├── requirements.txt           # Python依赖
├── pyproject.toml             # Python项目配置
└── README.md                  # 本文件
```

## 快速开始

### 1. 环境要求

- **C++**: GCC 9+/Clang 10+/MSVC 2019+
- **CMake**: 3.15+
- **Python**: 3.10+
- **数据库**: PostgreSQL 14+ (带TimescaleDB扩展), Redis 6+

### 2. 安装依赖

```bash
# 创建Python虚拟环境
python3 -m venv venv
source venv/bin/activate  # Linux/macOS
# 或 venv\Scripts\activate  # Windows

# 安装Python依赖
pip install -r requirements.txt

# 安装C++依赖 (macOS)
brew install cmake boost openssl nlohmann-json spdlog

# 安装C++依赖 (Ubuntu)
sudo apt-get install cmake libboost-all-dev libssl-dev nlohmann-json3-dev libspdlog-dev
```

### 3. 编译C++模块

```bash
./scripts/build.sh
```

### 4. 配置数据库

```bash
# 启动数据库 (使用Docker)
docker-compose up -d postgres redis

# 初始化数据库
python scripts/setup_db.py
```

### 5. 运行服务

```bash
# 开发模式
python -m python.api.main

# 生产模式
uvicorn python.api.main:app --host 0.0.0.0 --port 8000 --workers 4
```

## 开发指南

### C++ 模块开发

1. 在 `cpp/include/` 中定义头文件
2. 在 `cpp/src/` 中实现源文件
3. 在 `cpp/bindings/` 中添加Python绑定
4. 在 `cpp/tests/` 中添加测试
5. 运行 `./scripts/build.sh` 编译

### Python 模块开发

1. 遵循PEP 8代码规范
2. 使用类型提示
3. 添加docstring
4. 编写单元测试
5. 运行 `pytest` 测试

## 性能指标

- **数据采集延迟**: < 10ms
- **数据处理吞吐**: > 100,000 ticks/秒
- **API响应时间**: < 50ms (P99)
- **WebSocket推送延迟**: < 5ms

## 贡献指南

1. Fork 项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启Pull Request

## 许可证

MIT License

## 联系方式



