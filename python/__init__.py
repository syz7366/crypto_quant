"""
加密货币量化交易系统 - 数据层 Python模块
"""

__version__ = "1.0.0"
__author__ = "Your Name"

# 导入C++核心模块（注释掉自动导入，避免重复注册pybind11类型）
# try:
#     from . import quant_crypto_core
#     HAS_CPP_MODULE = True
# except ImportError:
#     HAS_CPP_MODULE = False
#     import warnings
#     warnings.warn("C++ core module not found. Please build it first.")

HAS_CPP_MODULE = True  # 假设模块已编译

