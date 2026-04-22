#ifndef ERROR_CODE_H
#define ERROR_CODE_H

#include <string>
#include <unordered_map>
#include <memory>

namespace chat {
namespace util {

enum class ErrorCode {
    SUCCESS = 0,
    
    SYSTEM_ERROR = 10000,
    INTERNAL_ERROR = 10001,
    TIMEOUT = 10002,
    SERVICE_UNAVAILABLE = 10003,
    MAINTENANCE = 10004,
    
    INVALID_PARAM = 20000,
    MISSING_PARAM = 20001,
    PARAM_TYPE_ERROR = 20002,
    PARAM_VALIDATION_FAILED = 20003,
    
    AUTH_ERROR = 30000,
    UNAUTHORIZED = 30001,
    TOKEN_EXPIRED = 30002,
    TOKEN_INVALID = 30003,
    PERMISSION_DENIED = 30004,
    
    USER_ERROR = 40000,
    USER_NOT_FOUND = 40001,
    USER_EXISTS = 40002,
    USER_OFFLINE = 40003,
    USER_BANNED = 40004,
    WRONG_PASSWORD = 40005,
    USER_NOT_LOGGED_IN = 40006,
    DUPLICATE_LOGIN = 40007,
    
    MESSAGE_ERROR = 50000,
    MESSAGE_NOT_FOUND = 50001,
    MESSAGE_TOO_LARGE = 50002,
    MESSAGE_SEND_FAILED = 50003,
    MESSAGE_EXPIRED = 50004,
    MESSAGE_DUPLICATE = 50005,
    
    GROUP_ERROR = 60000,
    GROUP_NOT_FOUND = 60001,
    GROUP_EXISTS = 60002,
    GROUP_MEMBER_LIMIT = 60003,
    NOT_GROUP_MEMBER = 60004,
    NOT_GROUP_ADMIN = 60005,
    GROUP_DISBANDED = 60006,
    USER_MUTED = 60007,
    
    FRIEND_ERROR = 70000,
    FRIEND_NOT_FOUND = 70001,
    FRIEND_REQUEST_EXISTS = 70002,
    ALREADY_FRIENDS = 70003,
    CANNOT_ADD_SELF = 70004,
    
    STORAGE_ERROR = 80000,
    DATABASE_ERROR = 80001,
    REDIS_ERROR = 80002,
    ETCD_ERROR = 80003,
    FILE_OPERATION_FAILED = 80004,
    
    NETWORK_ERROR = 90000,
    CONNECTION_FAILED = 90001,
    SSL_ERROR = 90002,
    PROTOCOL_ERROR = 90003
};

struct ErrorInfo {
    ErrorCode code;
    std::string message;
    std::string detail;
    bool should_log;
    bool should_notify_user;
    
    ErrorInfo() : code(ErrorCode::SUCCESS), should_log(false), should_notify_user(true) {}
    ErrorInfo(ErrorCode c, const std::string& msg, bool log = true, bool notify = true)
        : code(c), message(msg), should_log(log), should_notify_user(notify) {}
};

class ErrorCodeManager {
public:
    static ErrorCodeManager* instance();
    
    const ErrorInfo& getErrorInfo(ErrorCode code) const;
    std::string getErrorMessage(ErrorCode code) const;
    int getErrorCodeValue(ErrorCode code) const;
    
    void registerCustomError(ErrorCode code, const std::string& message);
    void overrideErrorMessage(ErrorCode code, const std::string& message);
    
    std::string toJson(ErrorCode code) const;
    std::string toJson(ErrorCode code, const std::string& detail) const;
    
private:
    ErrorCodeManager();
    
    std::unordered_map<ErrorCode, ErrorInfo> _errorMap;
    void initErrorMap();
};

class ErrorResult {
public:
    ErrorResult() : _code(ErrorCode::SUCCESS) {}
    ErrorResult(ErrorCode code) : _code(code) {}
    ErrorResult(ErrorCode code, const std::string& detail) 
        : _code(code), _detail(detail) {}
    
    ErrorCode code() const { return _code; }
    const std::string& message() const { return _message; }
    const std::string& detail() const { return _detail; }
    
    bool success() const { return _code == ErrorCode::SUCCESS; }
    bool failed() const { return !success(); }
    
    void setDetail(const std::string& detail) { _detail = detail; }
    
    std::string toJson() const;
    
private:
    ErrorCode _code;
    std::string _message;
    std::string _detail;
};

template<typename T>
class Result {
public:
    Result() : _error(ErrorCode::SUCCESS) {}
    Result(T&& value) : _value(std::move(value)), _error(ErrorCode::SUCCESS) {}
    Result(const T& value) : _value(value), _error(ErrorCode::SUCCESS) {}
    Result(ErrorCode error) : _error(error) {}
    Result(ErrorCode error, const std::string& detail) : _error(error), _detail(detail) {}
    
    bool success() const { return _error == ErrorCode::SUCCESS; }
    bool failed() const { return !success(); }
    
    const T& value() const { return _value; }
    T& value() { return _value; }
    
    ErrorCode error() const { return _error; }
    const std::string& errorMessage() const { return _errorMessage; }
    const std::string& detail() const { return _detail; }
    
    void setDetail(const std::string& detail) { _detail = detail; }
    
    std::string toJson() const;
    
private:
    T _value;
    ErrorCode _error;
    std::string _errorMessage;
    std::string _detail;
};

} // namespace util
} // namespace chat

#endif // ERROR_CODE_H