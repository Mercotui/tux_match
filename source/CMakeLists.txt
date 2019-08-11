#cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_VERBOSE_MAKEFILE OFF)

################################################################################
# Qt library
################################################################################
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 REQUIRED Core Gui Quick Widgets Multimedia )

################################################################################
# tux_match app
################################################################################
add_subdirectory(graphics_engine)
add_subdirectory(game_logic)

qt5_add_resources(tux_match_app_rcc ${tux_match_app_qml_qrc}
    # ${CMAKE_SOURCE_DIR}/resources/3D_models/3D_models.qrc
    ${CMAKE_SOURCE_DIR}/resources/app_resources.qrc)

set( tux_match_app_SOURCES   main.cpp ) #window.cpp)
#set( tux_match_app_HEADERS   window.hpp )
include_directories(AFTER SYSTEM src ${CMAKE_BINARY_DIR})

if(ANDROID)
    add_library(tux_match_app SHARED ${tux_match_app_SOURCES} ${tux_match_app_HEADERS} ${tux_match_app_rcc})
else()
    add_executable(tux_match_app ${tux_match_app_SOURCES} ${tux_match_app_HEADERS} ${tux_match_app_rcc})
endif()
target_link_libraries(tux_match_app graphics_engine Qt5::Core Qt5::Gui Qt5::Quick Qt5::Widgets Qt5::Multimedia )
set_target_properties( tux_match_app PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_PROJECT_BINARY_DIR}")
################################################################################
# Android apk creation
################################################################################

if(ANDROID)
    # Used by the qt-android-cmake project
    set( ANDROID_PLATFORM_LEVEL ${ANDROID_NATIVE_API_LEVEL})
    # Using the LLVM from the latest Android SDK (NDK v19)
    # This is used in the qt-android-cmake project
    set( ANDROID_USE_LLVM TRUE)

    # The STL lib to use
    set(ANDROID_STL_SHARED_LIBRARIES "c++_shared")

    include(../qt-android-cmake/AddQtAndroidApk.cmake)

    # Define the app
    add_qt_android_apk(
        tux_match_apk
        tux_match_app
        NAME "tux_match"
        # VERSION_CODE is single number
        # 10.25.1 => 10251
        VERSION_CODE "100"
        PACKAGE_NAME "org.derpicated.tux_match"
    )
endif() # ANDROID
target_compile_options(tux_match_app PRIVATE -std=c++17 -Wall -Wextra)

# copy over resource folder
add_custom_command(
        TARGET tux_match_app PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/resources/android_res
                ${CMAKE_CURRENT_BINARY_DIR}/package/res)

if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/package/AndroidManifest.xml")
    # read manifest
    file(READ ${CMAKE_CURRENT_BINARY_DIR}/package/AndroidManifest.xml ANDROID_MANIFEST_TMP)
    # add icon
    STRING(REGEX REPLACE
            "<application"
            "<application android:icon=\"@mipmap/ic_launcher\""
            ANDROID_MANIFEST_TMP
            ${ANDROID_MANIFEST_TMP})
    # write manifest
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/package/AndroidManifest.xml ${ANDROID_MANIFEST_TMP})
else()
endif()
