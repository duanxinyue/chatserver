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
    _errorMap[chatserver::ErrorCode::SUCCESS] = {chatserver::ErrorCode::SUCCESS, "success", false, false};
    
    _errorMap[chatserver::ErrorCode::SYSTEM_ERROR] = {chatserver::ErrorCode::SYSTEM_ERROR, "system error", true, true};
    _errorMap[chatserver::ErrorCode::INTERNAL_ERROR] = {chatserver::ErrorCode::INTERNAL_ERROR, "internal error", true, false};
    _errorMap[chatserver::ErrorCode::TIMEOUT] = {chatserver::ErrorCode::TIMEOUT, "request timeout", true, true};
    _errorMap[chatserver::ErrorCode::SERVICE_UNAVAILABLE] = {chatserver::ErrorCode::SERVICE_UNAVAILABLE, "service unavailable", true, true};
    _errorMap[chatserver::ErrorCode::MAINTENANCE] = {chatserver::ErrorCode::MAINTENANCE, "service under maintenance", true, true};
    
    _errorMap[chatserver::ErrorCode::INVALID_PARAM] = {chatserver::ErrorCode::INVALID_PARAM, "invalid parameter", true, true};
    _errorMap[chatserver::ErrorCode::MISSING_PARAM] = {chatserver::ErrorCode::MISSING_PARAM, "missing parameter", true, true};
    _errorMap[chatserver::ErrorCode::PARAM_TYPE_ERROR] = {chatserver::ErrorCode::PARAM_TYPE_ERROR, "parameter type error", true, true};
    _errorMap[chatserver::ErrorCode::PARAM_VALIDATION_FAILED] = {chatserver::ErrorCode::PARAM_VALIDATION_FAILED, "parameter validation failed", true, true};
    
    _errorMap[chatserver::ErrorCode::AUTH_ERROR] = {chatserver::ErrorCode::AUTH_ERROR, "authentication error", true, true};
    _errorMap[chatserver::ErrorCode::UNAUTHORIZED] = {chatserver::ErrorCode::UNAUTHORIZED, "unauthorized", true, true};
    _errorMap[chatserver::ErrorCode::TOKEN_EXPIRED] = {chatserver::ErrorCode::TOKEN_EXPIRED, "token expired", true, true};
    _errorMap[chatserver::ErrorCode::TOKEN_INVALID] = {chatserver::ErrorCode::TOKEN_INVALID, "invalid token", true, true};
    _errorMap[chatserver::ErrorCode::PERMISSION_DENIED] = {chatserver::ErrorCode::PERMISSION_DENIED, "permission denied", true, true};
    
    _errorMap[chatserver::ErrorCode::USER_ERROR] = {chatserver::ErrorCode::USER_ERROR, "user error", true, true};
    _errorMap[chatserver::ErrorCode::USER_NOT_FOUND] = {chatserver::ErrorCode::USER_NOT_FOUND, "user not found", true, true};
    _errorMap[chatserver::ErrorCode::USER_EXISTS] = {chatserver::ErrorCode::USER_EXISTS, "user already exists", true, true};
    _errorMap[chatserver::ErrorCode::USER_OFFLINE] = {chatserver::ErrorCode::USER_OFFLINE, "user offline", true, true};
    _errorMap[chatserver::ErrorCode::USER_BANNED] = {chatserver::ErrorCode::USER_BANNED, "user banned", true, true};
    _errorMap[chatserver::ErrorCode::WRONG_PASSWORD] = {chatserver::ErrorCode::WRONG_PASSWORD, "wrong password", true, true};
    _errorMap[chatserver::ErrorCode::USER_NOT_LOGGED_IN] = {chatserver::ErrorCode::USER_NOT_LOGGED_IN, "user not logged in", true, true};
    _errorMap[chatserver::ErrorCode::DUPLICATE_LOGIN] = {chatserver::ErrorCode::DUPLICATE_LOGIN, "duplicate login", true, true};
    
    _errorMap[chatserver::ErrorCode::MESSAGE_ERROR] = {chatserver::ErrorCode::MESSAGE_ERROR, "message error", true, true};
    _errorMap[chatserver::ErrorCode::MESSAGE_NOT_FOUND] = {chatserver::ErrorCode::MESSAGE_NOT_FOUND, "message not found", true, true};
    _errorMap[chatserver::ErrorCode::MESSAGE_TOO_LARGE] = {chatserver::ErrorCode::MESSAGE_TOO_LARGE, "message too large", true, true};
    _errorMap[chatserver::ErrorCode::MESSAGE_SEND_FAILED] = {chatserver::ErrorCode::MESSAGE_SEND_FAILED, "message send failed", true, true};
    _errorMap[chatserver::ErrorCode::MESSAGE_EXPIRED] = {chatserver::ErrorCode::MESSAGE_EXPIRED, "message expired", true, true};
    _errorMap[chatserver::ErrorCode::MESSAGE_DUPLICATE] = {chatserver::ErrorCode::MESSAGE_DUPLICATE, "duplicate message", true, true};
    
    _errorMap[chatserver::ErrorCode::GROUP_ERROR] = {chatserver::ErrorCode::GROUP_ERROR, "group error", true, true};
    _errorMap[chatserver::ErrorCode::GROUP_NOT_FOUND] = {chatserver::ErrorCode::GROUP_NOT_FOUND, "group not found", true, true};
    _errorMap[chatserver::ErrorCode::GROUP_EXISTS] = {chatserver::ErrorCode::GROUP_EXISTS, "group already exists", true, true};
    _errorMap[chatserver::ErrorCode::GROUP_MEMBER_LIMIT] = {chatserver::ErrorCode::GROUP_MEMBER_LIMIT, "group member limit reached", true, true};
    _errorMap[chatserver::ErrorCode::NOT_GROUP_MEMBER] = {chatserver::ErrorCode::NOT_GROUP_MEMBER, "not a group member", true, true};
    _errorMap[chatserver::ErrorCode::NOT_GROUP_ADMIN] = {chatserver::ErrorCode::NOT_GROUP_ADMIN, "not group admin", true, true};
    _errorMap[chatserver::ErrorCode::GROUP_DISBANDED] = {chatserver::ErrorCode::GROUP_DISBANDED, "group disbanded", true, true};
    _errorMap[chatserver::ErrorCode::USER_MUTED] = {chatserver::ErrorCode::USER_MUTED, "user muted", true, true};
    
    _errorMap[chatserver::ErrorCode::FRIEND_ERROR] = {chatserver::ErrorCode::FRIEND_ERROR, "friend error", true, true};
    _errorMap[chatserver::ErrorCode::FRIEND_NOT_FOUND] = {chatserver::ErrorCode::FRIEND_NOT_FOUND, "friend not found", true, true};
    _errorMap[chatserver::ErrorCode::FRIEND_REQUEST_EXISTS] = {chatserver::ErrorCode::FRIEND_REQUEST_EXISTS, "friend request already exists", true, true};
    _errorMap[chatserver::ErrorCode::ALREADY_FRIENDS] = {chatserver::ErrorCode::ALREADY_FRIENDS, "already friends", true, true};
    _errorMap[chatserver::ErrorCode::CANNOT_ADD_SELF] = {chatserver::ErrorCode::CANNOT_ADD_SELF, "cannot add self as friend", true, true};
    
    _errorMap[chatserver::ErrorCode::STORAGE_ERROR] = {chatserver::ErrorCode::STORAGE_ERROR, "storage error", true, true};
    _errorMap[chatserver::ErrorCode::DATABASE_ERROR] = {chatserver::ErrorCode::DATABASE_ERROR, "database error", true, false};
    _errorMap[chatserver::ErrorCode::REDIS_ERROR] = {chatserver::ErrorCode::REDIS_ERROR, "redis error", true, false};
    _errorMap[chatserver::ErrorCode::ETCD_ERROR] = {chatserver::ErrorCode::ETCD_ERROR, "etcd error", true, false};
    _errorMap[chatserver::ErrorCode::FILE_OPERATION_FAILED] = {chatserver::ErrorCode::FILE_OPERATION_FAILED, "file operation failed", true, false};
    
    _errorMap[chatserver::ErrorCode::NETWORK_ERROR] = {chatserver::ErrorCode::NETWORK_ERROR, "network error", true, true};
    _errorMap[chatserver::ErrorCode::CONNECTION_FAILED] = {chatserver::ErrorCode::CONNECTION_FAILED, "connection failed", true, true};
    _errorMap[chatserver::ErrorCode::SSL_ERROR] = {chatserver::ErrorCode::SSL_ERROR, "SSL error", true, false};
    _errorMap[chatserver::ErrorCode::PROTOCOL_ERROR] = {chatserver::ErrorCode::PROTOCOL_ERROR, "protocol error", true, true};
}

const ErrorInfo& ErrorCodeManager::getErrorInfo(chatserver::ErrorCode code) const {
    auto it = _errorMap.find(code);
    if (it != _errorMap.end()) {
        return it->second;
    }
    static ErrorInfo unknown = {chatserver::ErrorCode::SYSTEM_ERROR, "unknown error", true, true};
    return unknown;
}

std::string ErrorCodeManager::getErrorMessage(chatserver::ErrorCode code) const {
    return getErrorInfo(code).message;
}

int ErrorCodeManager::getErrorCodeValue(chatserver::ErrorCode code) const {
    return static_cast<int>(code);
}

void ErrorCodeManager::registerCustomError(chatserver::ErrorCode code, const std::string& message) {
    _errorMap[code] = {code, message, true, true};
}

void ErrorCodeManager::overrideErrorMessage(chatserver::ErrorCode code, const std::string& message) {
    auto it = _errorMap.find(code);
    if (it != _errorMap.end()) {
        it->second.message = message;
    }
}

std::string ErrorCodeManager::toJson(chatserver::ErrorCode code) const {
    return toJson(code, "");
}

std::string ErrorCodeManager::toJson(chatserver::ErrorCode code, const std::string& detail) const {
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
