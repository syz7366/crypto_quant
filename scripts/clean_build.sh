#!/bin/bash
# ============================================
# 清理并重新构建项目
# ============================================

set -e  # 遇到错误立即退出

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

echo "=========================================="
echo "🧹 清理旧的构建文件..."
echo "=========================================="
rm -rf build
mkdir -p build

echo ""
echo "=========================================="
echo "⚙️  开始CMake配置..."
echo "=========================================="
cd build
cmake ..

echo ""
echo "=========================================="
echo "🔨 开始编译..."
echo "=========================================="
make -j$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

echo ""
echo "=========================================="
echo "✅ 构建完成！"
echo "=========================================="
echo "Python模块位置: $PROJECT_ROOT/python/quant_crypto_core*.so"
ls -lh "$PROJECT_ROOT/python/quant_crypto_core"*.so 2>/dev/null || echo "注意：Python模块未生成"

