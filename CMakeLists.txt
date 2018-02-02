cmake_minimum_required(VERSION 3.2)
project(CBC)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenSSL REQUIRED)

set(CBC_FILES CBC.h CBC.cpp)

set(SOURCE_FILES ${CBC_FILES} main.cpp)

add_executable(CBC ${SOURCE_FILES})

target_link_libraries(CBC ${OPENSSL_LIBRARIES})
