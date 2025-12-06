#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "common/types.h"
#include "collectors/base_collector.h"
#include "normalizers/data_normalizer.h"
#include "cleaners/data_cleaner.h"
#include "collectors/binance_collector.h"
#include "storage/kline_storage.h"
#include "config/config_manager.h"
#include "strategy/strategy_base.h"
#include "strategy/ma_cross_strategy.h"
#include "backtest/backtest_engine.h"
#include "analysis/performance_metrics.h"
#include "analysis/performance_analyzer.h"
// #include "common/result.h"

namespace py = pybind11;
using namespace quant_crypto;

PYBIND11_MODULE(quant_crypto_core, m) {
    m.doc() = "高性能加密货币量化交易数据层C++核心模块";

    // ========== 枚举类型 ==========
    py::enum_<Timeframe>(m, "Timeframe")
        .value("TICK", Timeframe::TICK)
        .value("SECOND_1", Timeframe::SECOND_1)
        .value("MINUTE_1", Timeframe::MINUTE_1)
        .value("MINUTE_5", Timeframe::MINUTE_5)
        .value("MINUTE_15", Timeframe::MINUTE_15)
        .value("MINUTE_30", Timeframe::MINUTE_30)
        .value("HOUR_1", Timeframe::HOUR_1)
        .value("HOUR_4", Timeframe::HOUR_4)
        .value("DAY_1", Timeframe::DAY_1)
        .value("WEEK_1", Timeframe::WEEK_1)
        .value("MONTH_1", Timeframe::MONTH_1)
        .export_values();

    py::enum_<Side>(m, "Side")
        .value("BUY", Side::BUY)
        .value("SELL", Side::SELL)
        .value("UNKNOWN", Side::UNKNOWN)
        .export_values();

    py::enum_<OrderType>(m, "OrderType")
        .value("LIMIT", OrderType::LIMIT)
        .value("MARKET", OrderType::MARKET)
        .value("STOP_LIMIT", OrderType::STOP_LIMIT)
        .value("STOP_MARKET", OrderType::STOP_MARKET)
        .value("UNKNOWN", OrderType::UNKNOWN)
        .export_values();

    py::enum_<DataQuality>(m, "DataQuality")
        .value("GOOD", DataQuality::GOOD)
        .value("SUSPICIOUS", DataQuality::SUSPICIOUS)
        .value("BAD", DataQuality::BAD)
        .value("MISSING", DataQuality::MISSING)
        .export_values();

    // ========== 数据结构 ==========
    py::class_<OHLCV>(m, "OHLCV")
        .def(py::init<>())
        .def_readwrite("timestamp", &OHLCV::timestamp)
        .def_readwrite("symbol", &OHLCV::symbol)
        .def_readwrite("exchange", &OHLCV::exchange)
        .def_readwrite("timeframe", &OHLCV::timeframe)
        .def_readwrite("open", &OHLCV::open)
        .def_readwrite("high", &OHLCV::high)
        .def_readwrite("low", &OHLCV::low)
        .def_readwrite("close", &OHLCV::close)
        .def_readwrite("volume", &OHLCV::volume)
        .def_readwrite("quote_volume", &OHLCV::quote_volume)
        .def_readwrite("trades_count", &OHLCV::trades_count)
        .def_readwrite("quality", &OHLCV::quality)
        .def("__repr__", [](const OHLCV& ohlcv) {
            return "<OHLCV " + ohlcv.symbol + " " + ohlcv.exchange + 
                   " O:" + std::to_string(ohlcv.open) +
                   " H:" + std::to_string(ohlcv.high) +
                   " L:" + std::to_string(ohlcv.low) +
                   " C:" + std::to_string(ohlcv.close) +
                   " V:" + std::to_string(ohlcv.volume) + ">";
        });

    py::class_<Tick>(m, "Tick")
        .def(py::init<>())
        .def_readwrite("timestamp", &Tick::timestamp)
        .def_readwrite("symbol", &Tick::symbol)
        .def_readwrite("exchange", &Tick::exchange)
        .def_readwrite("price", &Tick::price)
        .def_readwrite("volume", &Tick::volume)
        .def_readwrite("side", &Tick::side)
        .def_readwrite("trade_id", &Tick::trade_id)
        .def("__repr__", [](const Tick& tick) {
            return "<Tick " + tick.symbol + " " + tick.exchange + 
                   " P:" + std::to_string(tick.price) +
                   " V:" + std::to_string(tick.volume) + ">";
        });

    py::class_<OrderBookLevel>(m, "OrderBookLevel")
        .def(py::init<>())
        .def(py::init<Price, Volume>())
        .def_readwrite("price", &OrderBookLevel::price)
        .def_readwrite("volume", &OrderBookLevel::volume);

    py::class_<OrderBook>(m, "OrderBook")
        .def(py::init<>())
        .def_readwrite("timestamp", &OrderBook::timestamp)
        .def_readwrite("symbol", &OrderBook::symbol)
        .def_readwrite("exchange", &OrderBook::exchange)
        .def_readwrite("bids", &OrderBook::bids)
        .def_readwrite("asks", &OrderBook::asks)
        .def_readwrite("sequence", &OrderBook::sequence);

    py::class_<Ticker>(m, "Ticker")
        .def(py::init<>())
        .def_readwrite("timestamp", &Ticker::timestamp)
        .def_readwrite("symbol", &Ticker::symbol)
        .def_readwrite("exchange", &Ticker::exchange)
        .def_readwrite("last", &Ticker::last)
        .def_readwrite("bid", &Ticker::bid)
        .def_readwrite("ask", &Ticker::ask)
        .def_readwrite("high_24h", &Ticker::high_24h)
        .def_readwrite("low_24h", &Ticker::low_24h)
        .def_readwrite("volume_24h", &Ticker::volume_24h)
        .def_readwrite("quote_volume_24h", &Ticker::quote_volume_24h)
        .def_readwrite("change_24h", &Ticker::change_24h);

    // ========== 工具函数 ==========
    m.def("timeframe_to_string", &timeframe_to_string, "将Timeframe枚举转为字符串");
    m.def("string_to_timeframe", &string_to_timeframe, "将字符串转为Timeframe枚举");
    m.def("timeframe_to_milliseconds", &timeframe_to_milliseconds, "获取时间周期的毫秒数");

    // ========== 数据标准化器 ==========
    py::class_<normalizers::DataNormalizer>(m, "DataNormalizer")
        .def(py::init<>())
        .def_static("normalize_symbol", &normalizers::DataNormalizer::normalize_symbol,
                   "标准化交易对名称")
        .def_static("denormalize_symbol", &normalizers::DataNormalizer::denormalize_symbol,
                   "反标准化交易对名称")
        .def_static("normalize_timestamp", &normalizers::DataNormalizer::normalize_timestamp,
                   "标准化时间戳",
                   py::arg("timestamp"), py::arg("unit") = "ms")
        .def_static("normalize_price", &normalizers::DataNormalizer::normalize_price,
                   "标准化价格",
                   py::arg("price"), py::arg("precision") = 8)
        .def_static("normalize_volume", &normalizers::DataNormalizer::normalize_volume,
                   "标准化成交量",
                   py::arg("volume"), py::arg("precision") = 8)
        .def_static("normalize_side", &normalizers::DataNormalizer::normalize_side,
                   "标准化订单方向")
        .def_static("normalize_timeframe", &normalizers::DataNormalizer::normalize_timeframe,
                   "标准化时间周期")
        .def_static("validate_ohlcv", &normalizers::DataNormalizer::validate_ohlcv,
                   "验证OHLCV数据")
        .def_static("validate_tick", &normalizers::DataNormalizer::validate_tick,
                   "验证Tick数据")
        .def_static("validate_orderbook", &normalizers::DataNormalizer::validate_orderbook,
                   "验证OrderBook数据");

    // ========== 数据清洗器 ==========
    py::class_<cleaners::DataCleaner>(m, "DataCleaner")
        .def(py::init<>())
        .def("clean_ohlcv", &cleaners::DataCleaner::clean_ohlcv,
             "清洗OHLCV数据")
        .def("clean_ohlcv_batch", &cleaners::DataCleaner::clean_ohlcv_batch,
             "批量清洗OHLCV数据")
        .def("clean_tick", &cleaners::DataCleaner::clean_tick,
             "清洗Tick数据")
        .def("clean_orderbook", &cleaners::DataCleaner::clean_orderbook,
             "清洗OrderBook数据")
        .def_static("detect_price_jump", &cleaners::DataCleaner::detect_price_jump,
                   "检测价格异常跳变",
                   py::arg("current"), py::arg("previous"), py::arg("threshold") = 0.5)
        .def_static("detect_volume_anomaly", &cleaners::DataCleaner::detect_volume_anomaly,
                   "检测成交量异常",
                   py::arg("volume"), py::arg("avg_volume"), py::arg("threshold") = 10.0)
        .def_static("is_timestamp_continuous", &cleaners::DataCleaner::is_timestamp_continuous,
                   "检查时间戳连续性",
                   py::arg("current_ts"), py::arg("previous_ts"),
                   py::arg("expected_interval"), py::arg("tolerance") = 1000)
        .def_static("deduplicate", &cleaners::DataCleaner::deduplicate,
                   "去除重复数据")
        .def_static("fill_missing", &cleaners::DataCleaner::fill_missing,
                   "填补缺失数据",
                   py::arg("data_list"), py::arg("timeframe"), py::arg("method") = "forward");

    py::class_<config::BinanceConfig>(m, "BinanceConfig")
        .def(py::init<>())
        .def_readwrite("base_url", &config::BinanceConfig::base_url)
        .def_readwrite("proxy_host", &config::BinanceConfig::proxy_host)
        .def_readwrite("proxy_port", &config::BinanceConfig::proxy_port)
        .def_readwrite("proxy_enabled", &config::BinanceConfig::proxy_enabled)
        .def_readwrite("timeout_ms", &config::BinanceConfig::timeout_ms)
        .def_readwrite("data_dir", &config::BinanceConfig::data_dir);
    
    py::class_<config::ConfigManager>(m, "ConfigManager")
        .def_static("load", &config::ConfigManager::load,
                   "加载配置文件", py::arg("config_file"))
        .def_static("get_binance_config", &config::ConfigManager::get_binance_config,
                   "获取币安配置");

    // ========== Result 模板特化 ==========
    // Result<vector<OHLCV>>
    py::class_<Result<std::vector<OHLCV>>>(m, "ResultOHLCVList")
        .def(py::init<>())
        .def_readwrite("success", &Result<std::vector<OHLCV>>::success)
        .def_readwrite("error_code", &Result<std::vector<OHLCV>>::error_code)
        .def_readwrite("error_message", &Result<std::vector<OHLCV>>::error_message)
        .def_readwrite("data", &Result<std::vector<OHLCV>>::data);

    // Result<Ticker>
    py::class_<Result<Ticker>>(m, "ResultTicker")
        .def(py::init<>())
        .def_readwrite("success", &Result<Ticker>::success)
        .def_readwrite("error_code", &Result<Ticker>::error_code)
        .def_readwrite("error_message", &Result<Ticker>::error_message)
        .def_readwrite("data", &Result<Ticker>::data);

    // Result<OrderBook>
    py::class_<Result<OrderBook>>(m, "ResultOrderBook")
        .def(py::init<>())
        .def_readwrite("success", &Result<OrderBook>::success)
        .def_readwrite("error_code", &Result<OrderBook>::error_code)
        .def_readwrite("error_message", &Result<OrderBook>::error_message)
        .def_readwrite("data", &Result<OrderBook>::data);
    
    // ========== BinanceCollector ==========
    py::class_<collectors::BinanceCollector>(m, "BinanceCollector")
        .def(py::init<const config::BinanceConfig&>(),
             "构造函数", py::arg("config"))
        .def("get_klines", &collectors::BinanceCollector::get_klines,
             "获取K线数据", py::arg("symbol"), py::arg("interval"), py::arg("limit") = 500)
        .def("get_ticker", &collectors::BinanceCollector::get_ticker,
             "获取24小时价格统计", py::arg("symbol"))
        .def("get_orderbook", &collectors::BinanceCollector::get_orderbook,
             "获取订单簿深度数据", py::arg("symbol"), py::arg("limit") = 100);
    
    // ========== 策略模块 ==========
    py::enum_<strategy::Signal>(m, "Signal")
        .value("BUY", strategy::Signal::BUY)
        .value("SELL", strategy::Signal::SELL)
        .value("HOLD", strategy::Signal::HOLD)
        .value("NONE", strategy::Signal::NONE)
        .export_values();

    py::class_<strategy::Position>(m, "Position")
        .def(py::init<>())
        .def_readwrite("symbol", &strategy::Position::symbol)
        .def_readwrite("quantity", &strategy::Position::quantity)
        .def_readwrite("avg_price", &strategy::Position::avg_price)
        .def_readwrite("current_price", &strategy::Position::current_price)
        .def_readwrite("unrealized_pnl", &strategy::Position::unrealized_pnl)
        .def("has_position", &strategy::Position::has_position);

    py::class_<strategy::Trade>(m, "Trade")
        .def(py::init<>())
        .def_readwrite("timestamp", &strategy::Trade::timestamp)
        .def_readwrite("symbol", &strategy::Trade::symbol)
        .def_readwrite("signal", &strategy::Trade::signal)
        .def_readwrite("price", &strategy::Trade::price)
        .def_readwrite("quantity", &strategy::Trade::quantity)
        .def_readwrite("pnl", &strategy::Trade::pnl);

    // 先绑定基类 StrategyBase
    py::class_<strategy::StrategyBase>(m, "StrategyBase")
        .def("on_init", &strategy::StrategyBase::on_init)
        .def("get_capital", &strategy::StrategyBase::get_capital)
        .def("get_total_equity", &strategy::StrategyBase::get_total_equity)
        .def("get_total_return", &strategy::StrategyBase::get_total_return)
        .def("get_position", &strategy::StrategyBase::get_position);

    py::class_<strategy::MACrossConfig>(m, "MACrossConfig")
        .def(py::init<>())
        .def_readwrite("fast_period", &strategy::MACrossConfig::fast_period)
        .def_readwrite("slow_period", &strategy::MACrossConfig::slow_period)
        .def_readwrite("position_size", &strategy::MACrossConfig::position_size);

    // 绑定派生类，指定继承关系
    py::class_<strategy::MACrossStrategy, strategy::StrategyBase>(m, "MACrossStrategy")
        .def(py::init<const strategy::MACrossConfig&>(),
             "构造函数", py::arg("config"))
        .def("on_bar", &strategy::MACrossStrategy::on_bar)
        .def("generate_signal", &strategy::MACrossStrategy::generate_signal)
        .def("get_name", &strategy::MACrossStrategy::get_name)
        .def("get_fast_ma", &strategy::MACrossStrategy::get_fast_ma)
        .def("get_slow_ma", &strategy::MACrossStrategy::get_slow_ma);

    // ========== 回测模块 ==========
    py::class_<backtest::BacktestConfig>(m, "BacktestConfig")
        .def(py::init<>())
        .def_readwrite("initial_capital", &backtest::BacktestConfig::initial_capital)
        .def_readwrite("commission_rate", &backtest::BacktestConfig::commission_rate)
        .def_readwrite("slippage_rate", &backtest::BacktestConfig::slippage_rate);

    py::class_<backtest::BacktestResult>(m, "BacktestResult")
        .def(py::init<>())
        .def_readwrite("initial_capital", &backtest::BacktestResult::initial_capital)
        .def_readwrite("final_capital", &backtest::BacktestResult::final_capital)
        .def_readwrite("final_equity", &backtest::BacktestResult::final_equity)
        .def_readwrite("total_return", &backtest::BacktestResult::total_return)
        .def_readwrite("total_trades", &backtest::BacktestResult::total_trades)
        .def_readwrite("winning_trades", &backtest::BacktestResult::winning_trades)
        .def_readwrite("losing_trades", &backtest::BacktestResult::losing_trades)
        .def_readwrite("trades", &backtest::BacktestResult::trades)
        .def_readwrite("equity_curve", &backtest::BacktestResult::equity_curve)
        .def_readwrite("timestamps", &backtest::BacktestResult::timestamps);

    py::class_<backtest::BacktestEngine>(m, "BacktestEngine")
        .def(py::init<const backtest::BacktestConfig&>(),
             "构造函数", py::arg("config"))
        .def("set_strategy", &backtest::BacktestEngine::set_strategy,
             "设置策略", py::arg("strategy"))
        .def("set_data", &backtest::BacktestEngine::set_data,
             "设置数据", py::arg("data"))
        .def("run", &backtest::BacktestEngine::run,
             "运行回测")
        .def("get_result", &backtest::BacktestEngine::get_result,
             "获取回测结果");

    // ========== 性能分析模块 ==========
    py::class_<analysis::PerformanceMetrics>(m, "PerformanceMetrics")
        .def(py::init<>())
        .def_readwrite("annualized_return", &analysis::PerformanceMetrics::annualized_return)
        .def_readwrite("cumulative_return", &analysis::PerformanceMetrics::cumulative_return)
        .def_readwrite("equity_curve", &analysis::PerformanceMetrics::equity_curve)
        .def_readwrite("max_drawdown", &analysis::PerformanceMetrics::max_drawdown)
        .def_readwrite("sharpe_ratio", &analysis::PerformanceMetrics::sharpe_ratio)
        .def_readwrite("sortino_ratio", &analysis::PerformanceMetrics::sortino_ratio)
        .def_readwrite("calmar_ratio", &analysis::PerformanceMetrics::calmar_ratio)
        .def_readwrite("volatility", &analysis::PerformanceMetrics::volatility)
        .def_readwrite("downside_deviation", &analysis::PerformanceMetrics::downside_deviation)
        .def_readwrite("profit_loss_ratio", &analysis::PerformanceMetrics::profit_loss_ratio)
        .def_readwrite("max_consecutive_wins", &analysis::PerformanceMetrics::max_consecutive_wins)
        .def_readwrite("max_consecutive_losses", &analysis::PerformanceMetrics::max_consecutive_losses)
        .def_readwrite("avg_holding_period", &analysis::PerformanceMetrics::avg_holding_period)
        .def_readwrite("trade_frequency_per_year", &analysis::PerformanceMetrics::trade_frequency_per_year)
        .def_readwrite("drawdown_curve", &analysis::PerformanceMetrics::drawdown_curve);

    py::class_<analysis::PerformanceAnalyzer>(m, "PerformanceAnalyzer")
        .def(py::init<>())
        .def("analyze", &analysis::PerformanceAnalyzer::analyze,
             "分析回测结果",
             py::arg("equity_curve"),
             py::arg("timestamps"),
             py::arg("trades"),
             py::arg("initial_captial"));


}

