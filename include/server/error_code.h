#ifndef ERROR_CODE_H
#define ERROR_CODE_H

#include <string>
#include <unordered_map>

namespace chat {

enum class ErrorCode {
    SUCCESS = 0,
    SYSTEM_ERROR = -1,
    INVALID_PARAMETER = -2,
    INVALID_USER_OR_PASSWORD = -3,
    USER_ALREADY_ONLINE = -4,
    REGISTER_FAILED = -5,
    INVALID_MESSAGE_TYPE = -6,
    USER_NOT_FOUND = -7,
    FRIEND_NOT_FOUND = -8,
    FRIEND_REQUEST_NOT_FOUND = -9,
    FRIEND_REQUEST_ALREADY_EXISTS = -10,
    FRIEND_REQUEST_REJECTED = -11,
    OFFLINE_MESSAGE_NOT_FOUND = -12,
    MESSAGE_DELIVERY_FAILED = -13,
    REDIS_ERROR = -14,
    MYSQL_ERROR = -15,
    
    INTERNAL_ERROR = -100,
    TIMEOUT = -101,
    SERVICE_UNAVAILABLE = -102,
    MAINTENANCE = -103,
    
    INVALID_PARAM = -200,
    MISSING_PARAM = -201,
    PARAM_TYPE_ERROR = -202,
    PARAM_VALIDATION_FAILED = -203,
    
    AUTH_ERROR = -300,
    UNAUTHORIZED = -301,
    TOKEN_EXPIRED = -302,
    TOKEN_INVALID = -303,
    PERMISSION_DENIED = -304,
    
    USER_ERROR = -400,
    USER_EXISTS = -401,
    USER_OFFLINE = -402,
    USER_BANNED = -403,
    WRONG_PASSWORD = -404,
    USER_NOT_LOGGED_IN = -405,
    DUPLICATE_LOGIN = -406,
    
    MESSAGE_ERROR = -500,
    MESSAGE_NOT_FOUND = -501,
    MESSAGE_TOO_LARGE = -502,
    MESSAGE_SEND_FAILED = -503,
    MESSAGE_EXPIRED = -504,
    MESSAGE_DUPLICATE = -505,
    
    GROUP_ERROR = -600,
    GROUP_NOT_FOUND = -601,
    GROUP_EXISTS = -602,
    GROUP_MEMBER_LIMIT = -603,
    NOT_GROUP_MEMBER = -604,
    NOT_GROUP_ADMIN = -605,
    GROUP_DISBANDED = -606,
    USER_MUTED = -607,
    
    FRIEND_ERROR = -700,
    FRIEND_REQUEST_EXISTS = -701,
    ALREADY_FRIENDS = -702,
    CANNOT_ADD_SELF = -703,
    
    STORAGE_ERROR = -800,
    DATABASE_ERROR = -801,
    ETCD_ERROR = -802,
    FILE_OPERATION_FAILED = -803,
    
    NETWORK_ERROR = -900,
    CONNECTION_FAILED = -901,
    SSL_ERROR = -902,
    PROTOCOL_ERROR = -903
};

struct ErrorInfo {
    ErrorCode code;
    std::string message;
    bool clientVisible;
    bool retryable;
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
    ErrorCodeManager(const ErrorCodeManager&) = delete;
    ErrorCodeManager& operator=(const ErrorCodeManager&) = delete;
    
    void initErrorMap();
    
    std::unordered_map<ErrorCode, ErrorInfo> _errorMap;
};

class ErrorResult {
public:
    ErrorResult(ErrorCode code, const std::string& detail = "") 
        : _code(code), _detail(detail) {}
    
    std::string toJson() const;
    
private:
    ErrorCode _code;
    std::string _detail;
};

template<typename T>
class Result {
public:
    Result() : _success(false), _error(ErrorCode::SYSTEM_ERROR) {}
    Result(const T& value) : _success(true), _value(value) {}
    Result(ErrorCode error, const std::string& detail = "") 
        : _success(false), _error(error), _detail(detail) {}
    
    bool success() const { return _success; }
    const T& value() const { return _value; }
    ErrorCode error() const { return _error; }
    const std::string& detail() const { return _detail; }
    
    std::string toJson() const;
    
private:
    bool _success;
    T _value;
    ErrorCode _error;
    std::string _detail;
};

} // namespace chat

#endif // ERROR_CODE_H
