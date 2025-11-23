#!/bin/bash
# ============================================
# 创建 VSCode/Cursor 的 C++ 配置文件
# ============================================

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

mkdir -p .vscode

# 创建 c_cpp_properties.json
cat > .vscode/c_cpp_properties.json <<'EOF'
{
    "configurations": [
        {
            "name": "Mac",
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/cpp/include",
                "${workspaceFolder}/third_party/httplib",
                "${workspaceFolder}/third_party/json",
                "${workspaceFolder}/third_party/pybind11/include",
                "/opt/homebrew/include",
                "/usr/local/include"
            ],
            "defines": [
                "CPPHTTPLIB_OPENSSL_SUPPORT"
            ],
            "macFrameworkPath": [
                "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks"
            ],
            "compilerPath": "/usr/bin/clang++",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "macos-clang-arm64",
            "compileCommands": "${workspaceFolder}/compile_commands.json"
        }
    ],
    "version": 4
}
EOF

echo "✓ 已创建 .vscode/c_cpp_properties.json"

# 创建 settings.json（推荐使用 clangd）
cat > .vscode/settings.json <<'EOF'
{
    "C_Cpp.default.compileCommands": "${workspaceFolder}/compile_commands.json",
    "C_Cpp.default.cppStandard": "c++17",
    "C_Cpp.errorSquiggles": "enabled",
    "clangd.arguments": [
        "--background-index",
        "--compile-commands-dir=${workspaceFolder}",
        "--clang-tidy",
        "--header-insertion=iwyu",
        "--completion-style=detailed",
        "--log=verbose"
    ],
    "files.associations": {
        "*.h": "cpp",
        "*.cpp": "cpp",
        "*.hpp": "cpp"
    }
}
EOF

echo "✓ 已创建 .vscode/settings.json"

# 创建 extensions.json（推荐安装的扩展）
cat > .vscode/extensions.json <<'EOF'
{
    "recommendations": [
        "llvm-vs-code-extensions.vscode-clangd",
        "ms-vscode.cpptools",
        "twxs.cmake",
        "ms-vscode.cmake-tools"
    ]
}
EOF

echo "✓ 已创建 .vscode/extensions.json"

echo ""
echo "=========================================="
echo "✅ VSCode/Cursor 配置文件已创建！"
echo "=========================================="
echo ""
echo "推荐安装的扩展："
echo "1. clangd (推荐) - 更强大的 C++ 智能提示"
echo "2. C/C++ (Microsoft) - 官方 C++ 扩展"
echo "3. CMake Tools - CMake 项目支持"

