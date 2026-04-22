#include "crypto.h"
#include "bcrypt.h"

namespace chat {
namespace util {

std::string Crypto::hash_password(const std::string& password) {
    BCrypt bcrypt;
    return bcrypt.hashPassword(password);
}

bool Crypto::verify_password(const std::string& password, const std::string& hash) {
    BCrypt bcrypt;
    return bcrypt.verifyPassword(password, hash);
}

std::string Crypto::generate_salt() {
    BCrypt bcrypt;
    std::string dummy_hash = bcrypt.hashPassword("dummy");
    size_t salt_end = dummy_hash.find_last_of('$');
    if (salt_end != std::string::npos) {
        return dummy_hash.substr(0, salt_end + 1);
    }
    return "$2a$12$";
}

bool Crypto::bcrypt_hash(const std::string& password, const std::string& salt, std::string& hash) {
    BCrypt bcrypt;
    hash = bcrypt.hashPassword(password);
    return !hash.empty();
}

bool Crypto::bcrypt_verify(const std::string& password, const std::string& hash) {
    BCrypt bcrypt;
    return bcrypt.verifyPassword(password, hash);
}

} // namespace util
} // namespace chat
