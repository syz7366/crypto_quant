#!/bin/bash
# ============================================
# 配置 IDE 智能提示和跳转功能
# ============================================

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

echo "=========================================="
echo "🔧 配置 IDE 智能功能..."
echo "=========================================="

# 1. 重新运行 CMake 生成 compile_commands.json
echo ""
echo "步骤 1/3: 生成编译数据库..."
cd build
cmake ..

# 2. 检查文件是否生成
if [ ! -f "compile_commands.json" ]; then
    echo "❌ 错误：compile_commands.json 未生成"
    exit 1
fi
echo "✓ compile_commands.json 已生成"

# 3. 创建符号链接到项目根目录（让 IDE 能找到）
echo ""
echo "步骤 2/3: 创建符号链接..."
cd "$PROJECT_ROOT"
if [ -L "compile_commands.json" ]; then
    rm compile_commands.json
fi
ln -s build/compile_commands.json compile_commands.json
echo "✓ 符号链接已创建: compile_commands.json -> build/compile_commands.json"

# 4. 检查 .vscode/c_cpp_properties.json（如果使用 VSCode）
echo ""
echo "步骤 3/3: 检查 IDE 配置..."
if [ -d ".vscode" ]; then
    echo "✓ 检测到 .vscode 目录"
    if [ ! -f ".vscode/c_cpp_properties.json" ]; then
        echo "⚠️  建议创建 .vscode/c_cpp_properties.json 配置文件"
        echo "   可以运行: ./scripts/create_vscode_config.sh"
    else
        echo "✓ c_cpp_properties.json 已存在"
    fi
else
    echo "ℹ️  未检测到 .vscode 目录（可能使用其他编辑器）"
fi

echo ""
echo "=========================================="
echo "✅ IDE 配置完成！"
echo "=========================================="
echo ""
echo "下一步操作："
echo "1. 重启你的 IDE（VSCode/Cursor/CLion 等）"
echo "2. 等待 IDE 重建索引（可能需要几秒到几分钟）"
echo "3. 尝试点击类名或函数名，应该可以跳转了！"
echo ""
echo "如果仍然无法跳转，请检查："
echo "- VSCode: 安装了 C/C++ 扩展或 clangd 扩展"
echo "- Cursor: 安装了 C/C++ 扩展"
echo "- CLion: 项目已正确加载为 CMake 项目"

