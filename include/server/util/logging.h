#ifndef LOGGING_H 
#define LOGGING_H 

#include <string> 
#include <memory> 
#include <sstream> 
#include <spdlog/spdlog.h> 
#include <spdlog/sinks/rotating_file_sink.h> 

namespace chat { 
namespace util { 

class Logger { 
public: 
    static Logger& instance(); 
    
    bool init(const std::string& log_dir, 
              const std::string& log_level, 
              size_t max_file_size, 
              int max_files); 
    
    std::shared_ptr<spdlog::logger>& get() { return _logger; } 
    
private: 
    Logger(); 
    ~Logger(); 
    
    Logger(const Logger&) = delete; 
    Logger& operator=(const Logger&) = delete; 
    
    std::shared_ptr<spdlog::logger> _logger; 
}; 

class LogStream { 
public: 
    std::shared_ptr<spdlog::logger> logger; 
    std::string level; 
    std::stringstream ss; 
    
    LogStream(std::shared_ptr<spdlog::logger> l, const std::string& lev) 
        : logger(l), level(lev) {} 
    
    ~LogStream() { 
        if (logger) { 
            if (level == "debug") logger->debug(ss.str()); 
            else if (level == "info") logger->info(ss.str()); 
            else if (level == "warn") logger->warn(ss.str()); 
            else if (level == "error") logger->error(ss.str()); 
        } 
    } 
    
    template<typename U> 
    LogStream& operator<<(const U& value) { 
        ss << value; 
        return *this; 
    } 
}; 

} // namespace util 
} // namespace chat 

#define LOG_DEBUG chat::util::LogStream(chat::util::Logger::instance().get(), "debug") 
#define LOG_INFO chat::util::LogStream(chat::util::Logger::instance().get(), "info") 
#define LOG_WARN chat::util::LogStream(chat::util::Logger::instance().get(), "warn") 
#define LOG_ERROR chat::util::LogStream(chat::util::Logger::instance().get(), "error") 

#endif // LOGGING_H 
