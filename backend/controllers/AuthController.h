#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

class AuthController : public drogon::HttpController<AuthController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(AuthController::googleSignIn, "/auth/google", Post);
    METHOD_LIST_END

        void googleSignIn(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
};
