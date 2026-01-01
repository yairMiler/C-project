cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(CMAKE_PREFIX_PATH "C:/vcpkg/installed/x64-windows")


project(CatanServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(backend)

