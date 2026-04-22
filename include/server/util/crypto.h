#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>

namespace chat {
namespace util {

class Crypto {
public:
    static std::string hash_password(const std::string& password);
    
    static bool verify_password(const std::string& password, const std::string& hash);
    
private:
    static std::string generate_salt();
    
    static bool bcrypt_hash(const std::string& password, const std::string& salt, std::string& hash);
    
    static bool bcrypt_verify(const std::string& password, const std::string& hash);
};

} // namespace util
} // namespace chat

#endif // CRYPTO_H
