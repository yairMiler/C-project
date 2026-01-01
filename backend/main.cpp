// main.cpp -- starts the drogon app
#define USE_POSTGRESQL true
#include <drogon/drogon.h>
#include <drogon/HttpAppFramework.h>
#include <functional> // Crucial for std::function
#include <iostream>
#include "C:\Games\Catan\backend\db\Db.h"




int main() {
    // 1. This loads the database and the server settings from the JSON
    // No more manual pgConfig!
    trantor::Logger::setLogLevel(trantor::Logger::kTrace);
    drogon::app().loadConfigFile("config.json");

    // 2. Add your CORS headers here
    drogon::app().registerPostHandlingAdvice([](const drogon::HttpRequestPtr &, const drogon::HttpResponsePtr &resp) {
        resp->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
        resp->addHeader("Access-Control-Allow-Credentials", "true");
    }).run();

    std::cout << "Server starting with config.json..." << std::endl;
    //drogon::app().run();
    //Db::init(drogon::app().getDbClient());
    return 0;
}

