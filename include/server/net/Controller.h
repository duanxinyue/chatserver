#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>
#include <memory>

namespace chat {
namespace net {

class Controller {
public:
    Controller(const std::string& conn_id);
    ~Controller();
    
    std::string remote_side() const;
    std::string connection_id() const;
    
    void response_attachment() const;
    void set_response_attachment(const std::string& data);
    
private:
    std::string conn_id_;
    mutable std::string response_data_;
};

} // namespace net
} // namespace chat

#endif // CONTROLLER_H
