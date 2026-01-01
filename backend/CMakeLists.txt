cmake_minimum_required(VERSION 3.16)
project(catan_backend LANGUAGES CXX)

find_package(PostgreSQL REQUIRED)
find_package(Drogon CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)

file(GLOB_RECURSE SRC 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
)

add_executable(catan_backend ${SRC})

target_link_libraries(catan_backend 
    PRIVATE PostgreSQL::PostgreSQL
    PRIVATE Drogon::Drogon 
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE libpqxx::pqxx
    PRIVATE OpenSSL::SSL
    PRIVATE jwt-cpp::jwt-cpp
)

add_custom_command(TARGET catan_backend POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/config.json"
        "$<TARGET_FILE_DIR:catan_backend>/config.json"
)
