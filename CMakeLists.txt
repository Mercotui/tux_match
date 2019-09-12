cmake_minimum_required(VERSION 2.8.11)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING
  "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel.")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
  Debug Release RelWithDebInfo MinSizeRel)

# options
option(ANDROID "switch to android build" OFF)

if(ANDROID)
  set(ANDROID_NATIVE_API_LEVEL "27" CACHE STRING
    "Choose the Android NDK Native APIs version to use.")
else(ANDROID)
endif(ANDROID)

include_directories(source)
add_subdirectory(source)
