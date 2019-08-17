cmake_minimum_required(VERSION 3.1.0)
project(picpic VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)


if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
elseif(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
endif()

find_package(Qt5 COMPONENTS Widgets Sql REQUIRED)

# Resouces

# TODO: create logo
set(ICO
    res/picpic.ico
)

# Build

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

if (WIN32)
    set(QT_WIN32_FIX WIN32)
endif ()

add_executable(picpic ${QT_WIN32_FIX} ${SOURCES})
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

# Install main binary
install(TARGETS
    picpic
    RUNTIME DESTINATION bin
)

# Install desktop file
if(UNIX)
    # TODO: create logo
    # install(FILES
    #     res/picpic.desktop
    #     DESTINATION share/applications
    # )
    # install(FILES
    #     res/picpic.png
    #     DESTINATION share/${PROJECT_NAME}
    # )
endif()


# Install dependencies
if (WIN32)
    # Install some Qt libraries
    set(QT_REQUIRED_LIBS_TARGET_NAMES
        Qt5::Widgets
        Qt5::Core
        Qt5::Gui
        Qt5::Sql
    )
    foreach(target_name ${QT_REQUIRED_LIBS_TARGET_NAMES})
        get_target_property(location ${target_name} LOCATION_Release)
        get_filename_component(dir ${location} DIRECTORY)
        get_filename_component(basename ${location} NAME_WE)
        file(GLOB all_locations ${dir}/${basename}.dll*)

        if(NOT all_locations)
            message(SEND_ERROR "Not found: ${target_name}")
        else()
            message(STATUS "Found ${target_name}: ${all_locations}")
            install(FILES
                ${all_locations}
                DESTINATION bin
            )
        endif()
    endforeach()

    # Install required Qt plugins
    get_filename_component(QT_ROOT ${Qt5_DIR}/../../.. ABSOLUTE)
    set(QT_PLATFORMS ${QT_ROOT}/plugins/platforms)
    set(QT_IMAGE_FORMATS ${QT_ROOT}/plugins/imageformats)
    set(QT_SQL_DRIVERS ${QT_ROOT}/plugins/sqldrivers)

    file(GLOB qsqlite_library "${QT_SQL_DRIVERS}/qsqlite.dll")
    if(NOT qsqlite_library)
        message(SEND_ERROR "Not found: qsqlite in ${QT_SQL_DRIVERS}")
    else()
        message(STATUS "Found qsqlite: ${qsqlite_library}")
    endif()
    install(FILES ${qsqlite_library} DESTINATION bin/sqldrivers)

    file(GLOB qjpeg_library "${QT_IMAGE_FORMATS}/qjpeg.dll")
    if(NOT qjpeg_library)
        message(SEND_ERROR "Not found: qjpeg in ${QT_IMAGE_FORMATS}")
    else()
        message(STATUS "Found qjpeg: ${qjpeg_library}")
    endif()
    install(FILES ${qjpeg_library} DESTINATION bin/imageformats)

    file(GLOB qico_library "${QT_IMAGE_FORMATS}/qico.dll")
    if(NOT qico_library)
        message(SEND_ERROR "Not found: qico in ${QT_IMAGE_FORMATS}")
    else()
        message(STATUS "Found qico: ${qico_library}")
    endif()
    install(FILES ${qico_library} DESTINATION bin/imageformats)

    file(GLOB qwindows_library "${QT_PLATFORMS}/qwindows.dll")
    if(NOT qwindows_library)
        message(SEND_ERROR "Not found: qwindows in ${QT_PLATFORMS}")
    else()
        message(STATUS "Found qjpeg: ${qwindows_library}")
    endif()
    install(FILES ${qwindows_library} DESTINATION bin/platforms)
endif()

# Package

set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "My own basic picture sorting software")
set(CPACK_PACKAGE_CONTACT "Quentin Chateau (quentin.chateau@gmail.com)")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")

if(UNIX)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5core5a, libqt5gui5, libqt5widgets5")
elseif(WIN32)
    set(CPACK_GENERATOR "NSIS")

    get_filename_component(ico_abspath ${ICO} REALPATH)
    string(REPLACE "/" "\\\\" ico_abspath ${ico_abspath})

    set(CPACK_PACKAGE_EXECUTABLES picpic picpic)
    set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${PROJECT_NAME}-${PROJECT_VERSION}")

    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        CreateShortCut \\\"$DESKTOP\\\\${PROJECT_NAME}.lnk\\\" \\\"$INSTDIR\\\\bin\\\\picpic.exe\\\"
    ")
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
        Delete \\\"$DESKTOP\\\\${PROJECT_NAME}.lnk\\\"
    ")

    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\picpic.exe")
    #set(CPACK_NSIS_MUI_ICON ${ico_abspath})
    #set(CPACK_NSIS_MUI_UNIICON ${ico_abspath})
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})
    set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME})
    set(CPACK_NSIS_PACKAGE_NAME ${PROJECT_NAME})
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/qchateau/picpic/")
    set(CPACK_NSIS_HELP_LINK "https://github.com/qchateau/picpic/")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
else ()
    message(FATAL_ERROR "Unsupported system")
endif()

include(CPack)