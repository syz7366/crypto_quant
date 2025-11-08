# 快速开始指南

## 前置要求

### 必需软件

1. **C++ 编译工具**
   ```bash
   # macOS
   xcode-select --install
   brew install cmake boost openssl nlohmann-json spdlog
   
   # Ubuntu/Debian
   sudo apt update
   sudo apt install build-essential cmake libboost-all-dev \
     libssl-dev nlohmann-json3-dev libspdlog-dev
   ```

2. **Python 3.10+**
   ```bash
   # macOS
   brew install python@3.10
   
   # Ubuntu/Debian
   sudo apt install python3.10 python3.10-venv python3.10-dev
   ```

3. **Docker & Docker Compose** (可选，用于数据库)
   - [Docker安装指南](https://docs.docker.com/get-docker/)

---

## 安装步骤

### 1. 克隆项目（已完成）

你已经在项目目录中了！

### 2. 安装pybind11

```bash
cd /Users/songhao/.cursor/worktrees/quant_crypto/mFppv
mkdir -p third_party
cd third_party
git clone https://github.com/pybind/pybind11.git
cd pybind11
git checkout v2.11.1
cd ../..
```

### 3. 创建Python虚拟环境

```bash
python3 -m venv venv
source venv/bin/activate  # macOS/Linux
# 或
# venv\Scripts\activate  # Windows
```

### 4. 安装Python依赖

```bash
pip install --upgrade pip
pip install -r requirements.txt
```

### 5. 编译C++模块

```bash
./scripts/build.sh
```

如果遇到权限问题：
```bash
chmod +x scripts/build.sh
./scripts/build.sh
```

### 6. 启动数据库（使用Docker）

```bash
cd docker
docker-compose up -d postgres redis
cd ..
```

检查服务状态：
```bash
docker-compose -f docker/docker-compose.yml ps
```

### 7. 初始化数据库

```bash
# 等待数据库启动（约10秒）
sleep 10

# 运行初始化脚本
python scripts/setup_db.py
```

### 8. 启动API服务

```bash
python -m python.api.main
```

你应该看到类似输出：
```
============================================================
🚀 加密货币量化交易数据层API启动中...
📝 文档地址: http://0.0.0.0:8000/docs
🔧 环境: development
============================================================
INFO:     Started server process
INFO:     Uvicorn running on http://0.0.0.0:8000
```

---

## 验证安装

### 1. 测试C++模块导入

```bash
python3 -c "
import python.quant_crypto_core as core
print('C++模块加载成功!')
print('可用类型:', dir(core))
"
```

### 2. 测试API健康检查

```bash
curl http://localhost:8000/api/v1/health
```

预期输出：
```json
{
  "status": "healthy",
  "timestamp": 1699999999999,
  "version": "1.0.0",
  "components": {
    "api": "ok",
    "database": "ok",
    "cache": "ok",
    "cpp_core": "ok"
  }
}
```

### 3. 查看API文档

打开浏览器访问：
- Swagger UI: http://localhost:8000/docs
- ReDoc: http://localhost:8000/redoc

---

## 下一步

### 开发第一个功能

1. **实现Binance数据采集器**
   - 编辑 `cpp/src/collectors/binance_collector.cpp`
   - 实现REST API和WebSocket连接
   - 添加Python绑定

2. **实现K线数据查询**
   - 编辑 `python/services/kline_service.py`
   - 实现数据库查询逻辑
   - 更新API路由

3. **添加测试**
   - 编写单元测试
   - 编写集成测试

### 配置交易所API

复制配置文件并填写API密钥：

```bash
cp .env.example .env
# 编辑 .env 文件，填写你的API密钥
```

---

## 常见问题

### Q1: CMake找不到Boost

**解决方案:**
```bash
# macOS
brew install boost

# Ubuntu
sudo apt install libboost-all-dev

# 或者手动指定路径
cmake .. -DBOOST_ROOT=/usr/local/opt/boost
```

### Q2: Python导入C++模块失败

**解决方案:**
```bash
# 检查模块是否编译成功
ls -la python/quant_crypto_core*.so

# 检查Python路径
python3 -c "import sys; print(sys.path)"

# 确保虚拟环境已激活
source venv/bin/activate
```

### Q3: 数据库连接失败

**解决方案:**
```bash
# 检查Docker服务
docker-compose -f docker/docker-compose.yml ps

# 查看日志
docker-compose -f docker/docker-compose.yml logs postgres

# 重启服务
docker-compose -f docker/docker-compose.yml restart postgres
```

### Q4: 端口已被占用

**解决方案:**
```bash
# 修改 .env 文件中的端口
echo "PORT=8001" >> .env

# 或者杀死占用端口的进程
lsof -ti:8000 | xargs kill -9
```

---

## 开发工具推荐

### 代码编辑器
- **VSCode** + C/C++扩展 + Python扩展
- **CLion** (C++开发)
- **PyCharm** (Python开发)

### 调试工具
- **gdb** (C++调试)
- **pdb** / **ipdb** (Python调试)
- **Postman** (API测试)

### 性能分析
- **valgrind** (内存分析)
- **perf** (性能分析)
- **py-spy** (Python性能分析)

---

## 获取帮助

- 查看 `docs/DEVELOPMENT.md` 了解详细开发指南
- 查看 `docs/API.md` 了解API使用说明
- 查看 `README.md` 了解项目架构

---

祝你开发顺利！🚀

