// Db.h
#pragma once
#include <drogon/orm/DbClient.h>

class Db {
public:
    static void init(const drogon::orm::DbClientPtr& client) {
        instance() = client;
    }

    static drogon::orm::DbClientPtr& get() {
        return instance();
    }

private:
    static drogon::orm::DbClientPtr& instance() {
        static drogon::orm::DbClientPtr client;
        return client;
    }
};
