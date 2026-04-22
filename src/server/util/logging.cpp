#include "logging.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <filesystem>
#include <iostream>

namespace chat {
namespace util {

Logger::Logger() : _logger(nullptr) {}

Logger::~Logger() {
    if (_logger) {
        _logger->flush();
    }
}

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

bool Logger::init(const std::string& log_dir,
                  const std::string& log_level,
                  size_t max_file_size,
                  int max_files) {
    try {
        std::filesystem::create_directories(log_dir);
        
        std::vector<spdlog::sink_ptr> sinks;
        
        auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        stdout_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        sinks.push_back(stdout_sink);
        
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            log_dir + "/chatserver.log",
            max_file_size,
            max_files
        );
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        sinks.push_back(file_sink);
        
        _logger = std::make_shared<spdlog::logger>("chatserver", sinks.begin(), sinks.end());
        
        spdlog::level::level_enum level = spdlog::level::info;
        if (log_level == "debug") {
            level = spdlog::level::debug;
        } else if (log_level == "warn") {
            level = spdlog::level::warn;
        } else if (log_level == "error") {
            level = spdlog::level::err;
        }
        _logger->set_level(level);
        
        _logger->flush_on(spdlog::level::err);
        
        _logger->info("Logger initialized: dir={}, level={}, max_file_size={}, max_files={}",
                     log_dir, log_level, max_file_size, max_files);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize logger: " << e.what() << std::endl;
        return false;
    }
}



} // namespace util
} // namespace chat
