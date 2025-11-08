#!/bin/bash

# 加密货币量化交易系统 - 构建脚本

set -e  # 遇到错误立即退出

echo "================================"
echo "🔨 开始构建项目..."
echo "================================"

# 获取脚本所在目录的父目录（项目根目录）
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查命令是否存在
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# 打印信息
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查必要工具
print_info "检查必要工具..."

if ! command_exists cmake; then
    print_error "CMake未安装，请先安装CMake"
    exit 1
fi

if ! command_exists python3; then
    print_error "Python3未安装，请先安装Python3"
    exit 1
fi

print_info "✓ 工具检查通过"

# 创建构建目录
print_info "创建构建目录..."
mkdir -p build
cd build

# 配置CMake
print_info "配置CMake..."

# 获取当前Python路径
PYTHON_EXE=$(which python)
print_info "使用Python: $PYTHON_EXE"

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=17 \
    -DPython_EXECUTABLE="$PYTHON_EXE" \
    || {
        print_error "CMake配置失败"
        exit 1
    }

# 编译
print_info "开始编译C++代码..."
CPU_CORES=$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)
cmake --build . --config Release -j"$CPU_CORES" || {
    print_error "编译失败"
    exit 1
}

print_info "✓ C++编译完成"

# 安装Python模块
cd "$PROJECT_ROOT"
print_info "安装Python依赖..."

# 检查是否在conda环境中
if [ -n "$CONDA_DEFAULT_ENV" ]; then
    print_info "检测到Conda环境: $CONDA_DEFAULT_ENV"
else
    # 检查虚拟环境
    if [ ! -d "venv" ]; then
        print_warning "虚拟环境不存在，创建虚拟环境..."
        python3 -m venv venv
    fi

    # 激活虚拟环境
    if [ -f "venv/bin/activate" ]; then
        source venv/bin/activate
    elif [ -f "venv/Scripts/activate" ]; then
        source venv/Scripts/activate
    fi
fi

# 升级pip
pip install --upgrade pip

# 安装依赖
pip install -r requirements.txt

print_info "✓ Python依赖安装完成"

# 测试C++模块导入
print_info "测试C++模块..."
cd "$PROJECT_ROOT"

# 使用编译时相同的Python
print_info "使用Python: $PYTHON_EXE"
"$PYTHON_EXE" -c "
import sys
sys.path.insert(0, '$PROJECT_ROOT')
try:
    import python.quant_crypto_core as core
    print('✓ C++模块导入成功')
    print('可用类型:', [x for x in dir(core) if not x.startswith('_')][:5], '...')
except ImportError as e:
    print(f'✗ C++模块导入失败: {e}')
    print(f'Python版本: {sys.version}')
    print(f'Python路径: {sys.path}')
    import os
    so_files = [f for f in os.listdir('$PROJECT_ROOT/python') if f.endswith('.so')]
    print(f'找到的.so文件: {so_files}')
    exit(1)
"

echo ""
echo "================================"
echo -e "${GREEN}✅ 构建完成！${NC}"
echo "================================"
echo ""
echo "下一步操作："
echo "  1. 配置数据库: python scripts/setup_db.py"
echo "  2. 启动服务: python -m python.api.main"
echo ""

