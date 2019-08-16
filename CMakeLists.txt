cmake_minimum_required(VERSION 3.12)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wpedantic -std=c++17")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(GUARDED_SOURCE_DIR ${CMAKE_BINARY_DIR}/libguarded)
set(GUARDED_INCLUDE_DIR ${GUARDED_SOURCE_DIR}/src)

find_package(Qt5 COMPONENTS Widgets Sql REQUIRED)

set(SOURCES
    main.cpp
    mainwindow.cpp
    image_viewer.cpp
    file_view.cpp
    mainwindow.hpp
    file_scanner.hpp
    database.hpp
    image_viewer.hpp
    file_view.hpp
)

add_executable(picpic ${SOURCES})
target_link_libraries(
    picpic
    Qt5::Widgets
    Qt5::Sql
)
target_include_directories(
    picpic
    PUBLIC
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Sql_INCLUDE_DIRS}
    ${GUARDED_INCLUDE_DIR}
)
