#!/bin/bash

# 安装pybind11脚本

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

echo "📦 安装pybind11..."

# 创建third_party目录
mkdir -p third_party
cd third_party

# 如果已经存在，先删除
if [ -d "pybind11" ]; then
    echo "⚠️  pybind11已存在，删除旧版本..."
    rm -rf pybind11
fi

# 克隆pybind11
echo "📥 下载pybind11..."
git clone https://github.com/pybind/pybind11.git

cd pybind11

# 切换到稳定版本
echo "🔀 切换到v2.11.1版本..."
git checkout v2.11.1

cd ../..

echo "✅ pybind11安装完成！"
echo ""
echo "现在可以运行: ./scripts/build.sh"

