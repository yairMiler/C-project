// JwtVerifier.cpp
#include "JwtVerifier.h"
#include <drogon/HttpClient.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <iostream>
#include <ctime> // For time(nullptr)

using namespace drogon;

JwtVerifier::JwtVerifier(const std::string& audience) : audience_(audience) {}

std::optional<std::map<std::string, std::string>> JwtVerifier::verify(const std::string& id_token)
{
    auto client = HttpClient::newHttpClient("https://oauth2.googleapis.com");

    auto req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath("/tokeninfo?id_token=" + id_token);

    // Perform synchronous request. The return type is std::pair<drogon::ReqResult, drogon::HttpResponsePtr>
    auto responsePair = client->sendRequest(req);

    auto resp = responsePair.second; // This is the actual HttpResponsePtr

    // This check is standard and robust across versions:
    if (!resp || resp->getStatusCode() != k200OK) {
        // k200OK is a standard Drogon status code enum member
        return std::nullopt;
    }

    // resp->body() is the JSON payload from Google (returns a std::string)
    try {
        auto j = json::parse(resp->getBody());

        if (!j.contains("aud")) return std::nullopt;
        if (j["aud"].get<std::string>() != audience_) return std::nullopt;

        if (j.contains("iss")) {
            std::string iss = j["iss"].get<std::string>();
            if (iss != "https://accounts.google.com" && iss != "accounts.google.com") {
                return std::nullopt;
            }
        }

        if (j.contains("exp")) {
            long exp = j["exp"].get<long>();
            long now = time(nullptr);
            if (exp < now) return std::nullopt;
        }

        std::map<std::string, std::string> claims;
        for (auto it = j.begin(); it != j.end(); ++it) {
            if (it.value().is_string()) {
                claims[it.key()] = it.value().get<std::string>();
            }
            else {
                claims[it.key()] = it.value().dump();
            }
        }
        return claims;
    }
    catch (std::exception& ex) {
        std::cerr << "JSON parsing error in JwtVerifier: " << ex.what() << std::endl;
        return std::nullopt;
    }
}


