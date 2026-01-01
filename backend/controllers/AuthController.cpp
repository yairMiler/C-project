#include "AuthController.h"
#include <drogon/drogon.h>
#include "../util/JwtVerifier.h"
#include "../db/Repository.h"
#include <nlohmann/json.hpp>
#include <sstream> // Required for ostringstream

using namespace drogon;

using json = nlohmann::json;

// Implementation of the class structure that was in the prompt but likely belongs in .h
// class AuthController : public drogon::HttpController<AuthController> { ... };

void AuthController::googleSignIn(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
    auto body = req->getJsonObject(); 

    
    if (!body || !body->isMember("id_token")) {
        
        Json::Value errorResp;
        errorResp["error"] = "missing id_token";
        callback(HttpResponse::newHttpJsonResponse(errorResp));
        return;
    }

    
    std::string id_token = (*body)["id_token"].asString();

    auto& conf = drogon::app().getCustomConfig();

    std::string clientId;
    if (conf.isMember("google") && conf["google"].isMember("client_id")) {
        clientId = conf["google"]["client_id"].asString();}
    else { throw std::runtime_error("google.client_id missing in config.json");}

    JwtVerifier verifier(clientId);
    auto claimsOpt = verifier.verify(id_token);
    if (!claimsOpt.has_value()) {
        Json::Value errorResp;
        errorResp["error"] = "invalid token";
        callback(HttpResponse::newHttpJsonResponse(errorResp));
        return;
    }

    auto claims = claimsOpt.value(); 

    std::string googleId = claims["sub"];

   
    std::string name = (claims.find("name") != claims.end()) ? claims["name"] : "Anonymous";

 
    drogon::orm::DbClientPtr clientPtr = drogon::app().getDbClient();
    Repository repo(clientPtr);

    int userId = repo.createUserIfNotExists(googleId, name);


    json out_nlohmann;
    out_nlohmann["token"] = id_token; 
    out_nlohmann["user"] = { {"id", userId}, {"display_name", name} };

    // Convert nlohmann::json back to Json::Value for the Drogon response
    Json::Value jsoncpp_response;
    Json::Reader reader;
    reader.parse(out_nlohmann.dump(), jsoncpp_response);

    
    callback(HttpResponse::newHttpJsonResponse(jsoncpp_response));
}
