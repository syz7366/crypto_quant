#include "storage/kline_storage.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <sys/stat.h>

namespace quant_crypto {
namespace storage {

    KlineStorage::KlineStorage(const std::string& data_dir) : data_dir_(data_dir) {
        // 创建数据目录（如果不存在）
        if (!data_dir_.empty() && data_dir_.back() != '/'){
            data_dir_ += '/';
        }
        int result = mkdir(data_dir_.c_str(), 0755);
        // 检查是否创建成功
        if (result == 0) {
            std::cout << "[KlineStorage] ✓ 成功创建数据目录: " << data_dir_ << std::endl;
        } else if (errno == EEXIST) {
            std::cout << "[KlineStorage] ✓ 数据目录已存在: " << data_dir_ << std::endl;
        } else {
            std::cerr << "[KlineStorage] ⚠️ 创建目录失败，错误码: " << errno << std::endl;
            std::cerr << "[KlineStorage] 请手动创建目录: mkdir -p " << data_dir_ << std::endl;
        }
        std::cout<< "[KlineStorage] 初始化完成，数据目录: " << data_dir_ << std::endl;
    }

    std::string KlineStorage::generate_filename(const std::string& symbol, const std::string& interval) const{
        std::time_t now = std::time(nullptr);
        std::tm* tm_now = std::localtime(&now);

        // 格式化日期字符串
        std::ostringstream date_stream;
        date_stream << std::put_time(tm_now, "%Y%m%d");
        std::string date_str = date_stream.str();

        // 拼接文件名： symbol_interval_date.csv
        std::string filename = data_dir_ + symbol + "_" + interval + "_" + date_str + ".csv";
        return filename;
    }

    bool KlineStorage::save_ohlcv(const std::string& symbol, const std::string& interval, const std::vector<OHLCV>& ohlcv_list){
        //1. 生成文件名
        std::string filename = generate_filename(symbol, interval);

        std::cout<< "[KlineStorage] 保存OHLCV数据到："<<filename<<std::endl;
        //2. 检查文件是否已经存在
        bool file_exists = false;
        {
            std::ifstream file(filename);
            file_exists = file.good();
        }
        //3. 打开文件（追加模式) 
        std::ofstream file(filename, std::ios::app);

        if(!file.is_open()){
            std::cerr << "[KlineStorage] 无法打开文件: " << filename << std::endl;
            return false;
        }

        // 4. 如果是新文件，写入CSV表头
        if(!file_exists){
            file << "timestamp,symbol,exchange,timeframe,open,high,low,close,volume,quote_volume,trades_count,quality" << std::endl;
            std::cout << "[KlineStorage] 新文件，已写入CSV表头" << std::endl;
        }
        // 5. 遍历OHLCV数据，写入每一行
        for (const auto& ohlcv : ohlcv_list) {
            file << ohlcv.timestamp << ","
                    << ohlcv.symbol << ","
                    << ohlcv.exchange << ","
                    << timeframe_to_string(ohlcv.timeframe) << ","
                    << std::fixed << std::setprecision(8) << ohlcv.open << ","
                    << ohlcv.high << ","
                    << ohlcv.low << ","
                    << ohlcv.close << ","
                    << ohlcv.volume << ","
                    << ohlcv.quote_volume << ","
                    << ohlcv.trades_count << ","
                    << static_cast<int>(ohlcv.quality) << std::endl;
        }

        // 6.关闭文件
        file.close();
        std::cout << "[KlineStorage] 保存完成，" << ohlcv_list.size() << " 条OHLCV数据" << std::endl;
        // 7.返回成功
        return true;
    }



}
}

