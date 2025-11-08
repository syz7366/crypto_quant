"""
加密货币量化交易系统 - 数据层 Python模块
"""

__version__ = "1.0.0"
__author__ = "Your Name"

# 导入C++核心模块
try:
    from . import quant_crypto_core
    HAS_CPP_MODULE = True
except ImportError:
    HAS_CPP_MODULE = False
    import warnings
    warnings.warn("C++ core module not found. Please build it first.")

