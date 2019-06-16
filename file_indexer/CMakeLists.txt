cmake_minimum_required(VERSION 3.12)

set(LOGLEVEL ERROR CACHE STRING "log level: TRACE DEBUG INFO WARN ERROR CRITICAL OFF")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wpedantic -std=c++17")
add_definitions(-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_${LOGLEVEL})

file(GLOB SOURCES *.cpp)

find_package(Boost REQUIRED COMPONENTS system program_options)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

set(SPDLOG_SOURCE_DIR ${CMAKE_BINARY_DIR}/spdlog)
set(SPDLOG_INCLUDE_DIRS ${SPDLOG_SOURCE_DIR}/include)

include(ExternalProject)
ExternalProject_Add(
    spdlog
    SOURCE_DIR ${SPDLOG_SOURCE_DIR}
    GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
    GIT_TAG "v1.3.1"
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "true"
)

add_executable(file_indexer
    ${SOURCES}
)
target_link_libraries(
    file_indexer
    stdc++fs
    OpenSSL::Crypto
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)
target_include_directories(
    file_indexer
    PUBLIC
    ${Boost_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
)