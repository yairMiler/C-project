#pragma once
#include <string>
#include <optional>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// A minimal verifier that fetches Google's JWKS (public keys),
// decodes and verifies the JWT signature via jwt-cpp (openssl).
// Requires jwt-cpp and openssl available through vcpkg.
class JwtVerifier {
public:
    JwtVerifier(const std::string& audience);
    // returns optional map of claims (string->string)
    std::optional<std::map<std::string, std::string>> verify(const std::string& id_token);
private:
    std::string audience_;
    // cache JWKS keys in-memory
};
