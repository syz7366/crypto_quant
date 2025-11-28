#pragma once
#include <cstdint>

namespace quant_crypto {
namespace common {

struct Kline{
    uint64_t open_time;//时间戳
    double open;//开盘价
    double high; // 最高价
    double low;// 最低价
    double close;// 收盘价
    double volume; // 成交量
};

}
}



