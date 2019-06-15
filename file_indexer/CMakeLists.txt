cmake_minimum_required(VERSION 3.12)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++17")

file(GLOB SOURCES *.cpp)

add_executable(file_indexer
    ${SOURCES}
)
target_link_libraries(file_indexer stdc++fs crypto)
target_include_directories(file_indexer PUBLIC ../third_party)