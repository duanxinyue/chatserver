#include "Controller.h"

namespace chat {
namespace net {

Controller::Controller(const std::string& conn_id) : conn_id_(conn_id) {}

Controller::~Controller() {}

std::string Controller::remote_side() const {
    return conn_id_;
}

std::string Controller::connection_id() const {
    return conn_id_;
}

void Controller::response_attachment() const {
}

void Controller::set_response_attachment(const std::string& data) {
    response_data_ = data;
}

} // namespace net
} // namespace chat
