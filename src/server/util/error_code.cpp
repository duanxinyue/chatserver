#include "error_code.h"
#include "json.hpp"
#include <sstream>

using json = nlohmann::json;

namespace chat {

ErrorCodeManager::ErrorCodeManager() {
    initErrorMap();
}

ErrorCodeManager* ErrorCodeManager::instance() {
    static ErrorCodeManager instance;
    return &instance;
}

void ErrorCodeManager::initErrorMap() {
    _errorMap[ErrorCode::SUCCESS] = {ErrorCode::SUCCESS, "success", false, false};
    
    _errorMap[ErrorCode::SYSTEM_ERROR] = {ErrorCode::SYSTEM_ERROR, "system error", true, true};
    _errorMap[ErrorCode::INTERNAL_ERROR] = {ErrorCode::INTERNAL_ERROR, "internal error", true, false};
    _errorMap[ErrorCode::TIMEOUT] = {ErrorCode::TIMEOUT, "request timeout", true, true};
    _errorMap[ErrorCode::SERVICE_UNAVAILABLE] = {ErrorCode::SERVICE_UNAVAILABLE, "service unavailable", true, true};
    _errorMap[ErrorCode::MAINTENANCE] = {ErrorCode::MAINTENANCE, "service under maintenance", true, true};
    
    _errorMap[ErrorCode::INVALID_PARAM] = {ErrorCode::INVALID_PARAM, "invalid parameter", true, true};
    _errorMap[ErrorCode::MISSING_PARAM] = {ErrorCode::MISSING_PARAM, "missing parameter", true, true};
    _errorMap[ErrorCode::PARAM_TYPE_ERROR] = {ErrorCode::PARAM_TYPE_ERROR, "parameter type error", true, true};
    _errorMap[ErrorCode::PARAM_VALIDATION_FAILED] = {ErrorCode::PARAM_VALIDATION_FAILED, "parameter validation failed", true, true};
    
    _errorMap[ErrorCode::AUTH_ERROR] = {ErrorCode::AUTH_ERROR, "authentication error", true, true};
    _errorMap[ErrorCode::UNAUTHORIZED] = {ErrorCode::UNAUTHORIZED, "unauthorized", true, true};
    _errorMap[ErrorCode::TOKEN_EXPIRED] = {ErrorCode::TOKEN_EXPIRED, "token expired", true, true};
    _errorMap[ErrorCode::TOKEN_INVALID] = {ErrorCode::TOKEN_INVALID, "invalid token", true, true};
    _errorMap[ErrorCode::PERMISSION_DENIED] = {ErrorCode::PERMISSION_DENIED, "permission denied", true, true};
    
    _errorMap[ErrorCode::USER_ERROR] = {ErrorCode::USER_ERROR, "user error", true, true};
    _errorMap[ErrorCode::USER_NOT_FOUND] = {ErrorCode::USER_NOT_FOUND, "user not found", true, true};
    _errorMap[ErrorCode::USER_EXISTS] = {ErrorCode::USER_EXISTS, "user already exists", true, true};
    _errorMap[ErrorCode::USER_OFFLINE] = {ErrorCode::USER_OFFLINE, "user offline", true, true};
    _errorMap[ErrorCode::USER_BANNED] = {ErrorCode::USER_BANNED, "user banned", true, true};
    _errorMap[ErrorCode::WRONG_PASSWORD] = {ErrorCode::WRONG_PASSWORD, "wrong password", true, true};
    _errorMap[ErrorCode::USER_NOT_LOGGED_IN] = {ErrorCode::USER_NOT_LOGGED_IN, "user not logged in", true, true};
    _errorMap[ErrorCode::DUPLICATE_LOGIN] = {ErrorCode::DUPLICATE_LOGIN, "duplicate login", true, true};
    
    _errorMap[ErrorCode::MESSAGE_ERROR] = {ErrorCode::MESSAGE_ERROR, "message error", true, true};
    _errorMap[ErrorCode::MESSAGE_NOT_FOUND] = {ErrorCode::MESSAGE_NOT_FOUND, "message not found", true, true};
    _errorMap[ErrorCode::MESSAGE_TOO_LARGE] = {ErrorCode::MESSAGE_TOO_LARGE, "message too large", true, true};
    _errorMap[ErrorCode::MESSAGE_SEND_FAILED] = {ErrorCode::MESSAGE_SEND_FAILED, "message send failed", true, true};
    _errorMap[ErrorCode::MESSAGE_EXPIRED] = {ErrorCode::MESSAGE_EXPIRED, "message expired", true, true};
    _errorMap[ErrorCode::MESSAGE_DUPLICATE] = {ErrorCode::MESSAGE_DUPLICATE, "duplicate message", true, true};
    
    _errorMap[ErrorCode::GROUP_ERROR] = {ErrorCode::GROUP_ERROR, "group error", true, true};
    _errorMap[ErrorCode::GROUP_NOT_FOUND] = {ErrorCode::GROUP_NOT_FOUND, "group not found", true, true};
    _errorMap[ErrorCode::GROUP_EXISTS] = {ErrorCode::GROUP_EXISTS, "group already exists", true, true};
    _errorMap[ErrorCode::GROUP_MEMBER_LIMIT] = {ErrorCode::GROUP_MEMBER_LIMIT, "group member limit reached", true, true};
    _errorMap[ErrorCode::NOT_GROUP_MEMBER] = {ErrorCode::NOT_GROUP_MEMBER, "not a group member", true, true};
    _errorMap[ErrorCode::NOT_GROUP_ADMIN] = {ErrorCode::NOT_GROUP_ADMIN, "not group admin", true, true};
    _errorMap[ErrorCode::GROUP_DISBANDED] = {ErrorCode::GROUP_DISBANDED, "group disbanded", true, true};
    _errorMap[ErrorCode::USER_MUTED] = {ErrorCode::USER_MUTED, "user muted", true, true};
    
    _errorMap[ErrorCode::FRIEND_ERROR] = {ErrorCode::FRIEND_ERROR, "friend error", true, true};
    _errorMap[ErrorCode::FRIEND_NOT_FOUND] = {ErrorCode::FRIEND_NOT_FOUND, "friend not found", true, true};
    _errorMap[ErrorCode::FRIEND_REQUEST_EXISTS] = {ErrorCode::FRIEND_REQUEST_EXISTS, "friend request already exists", true, true};
    _errorMap[ErrorCode::ALREADY_FRIENDS] = {ErrorCode::ALREADY_FRIENDS, "already friends", true, true};
    _errorMap[ErrorCode::CANNOT_ADD_SELF] = {ErrorCode::CANNOT_ADD_SELF, "cannot add self as friend", true, true};
    
    _errorMap[ErrorCode::STORAGE_ERROR] = {ErrorCode::STORAGE_ERROR, "storage error", true, true};
    _errorMap[ErrorCode::DATABASE_ERROR] = {ErrorCode::DATABASE_ERROR, "database error", true, false};
    _errorMap[ErrorCode::REDIS_ERROR] = {ErrorCode::REDIS_ERROR, "redis error", true, false};
    _errorMap[ErrorCode::ETCD_ERROR] = {ErrorCode::ETCD_ERROR, "etcd error", true, false};
    _errorMap[ErrorCode::FILE_OPERATION_FAILED] = {ErrorCode::FILE_OPERATION_FAILED, "file operation failed", true, false};
    
    _errorMap[ErrorCode::NETWORK_ERROR] = {ErrorCode::NETWORK_ERROR, "network error", true, true};
    _errorMap[ErrorCode::CONNECTION_FAILED] = {ErrorCode::CONNECTION_FAILED, "connection failed", true, true};
    _errorMap[ErrorCode::SSL_ERROR] = {ErrorCode::SSL_ERROR, "SSL error", true, false};
    _errorMap[ErrorCode::PROTOCOL_ERROR] = {ErrorCode::PROTOCOL_ERROR, "protocol error", true, true};
}

const ErrorInfo& ErrorCodeManager::getErrorInfo(ErrorCode code) const {
    auto it = _errorMap.find(code);
    if (it != _errorMap.end()) {
        return it->second;
    }
    static ErrorInfo unknown = {ErrorCode::SYSTEM_ERROR, "unknown error", true, true};
    return unknown;
}

std::string ErrorCodeManager::getErrorMessage(ErrorCode code) const {
    return getErrorInfo(code).message;
}

int ErrorCodeManager::getErrorCodeValue(ErrorCode code) const {
    return static_cast<int>(code);
}

void ErrorCodeManager::registerCustomError(ErrorCode code, const std::string& message) {
    _errorMap[code] = {code, message, true, true};
}

void ErrorCodeManager::overrideErrorMessage(ErrorCode code, const std::string& message) {
    auto it = _errorMap.find(code);
    if (it != _errorMap.end()) {
        it->second.message = message;
    }
}

std::string ErrorCodeManager::toJson(ErrorCode code) const {
    return toJson(code, "");
}

std::string ErrorCodeManager::toJson(ErrorCode code, const std::string& detail) const {
    const ErrorInfo& info = getErrorInfo(code);
    json j;
    j["code"] = static_cast<int>(info.code);
    j["message"] = info.message;
    if (!detail.empty()) {
        j["detail"] = detail;
    }
    return j.dump();
}

std::string ErrorResult::toJson() const {
    return ErrorCodeManager::instance()->toJson(_code, _detail);
}

template<typename T>
std::string Result<T>::toJson() const {
    json j;
    if (success()) {
        j["code"] = 0;
        j["message"] = "success";
        j["data"] = _value;
    } else {
        j["code"] = static_cast<int>(_error);
        j["message"] = ErrorCodeManager::instance()->getErrorMessage(_error);
        if (!_detail.empty()) {
            j["detail"] = _detail;
        }
    }
    return j.dump();
}

} // namespace chat
