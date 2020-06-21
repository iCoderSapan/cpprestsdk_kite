cmake_minimum_required(VERSION 3.9)

project(LibKiteConnect)

# Project targets
set(TARGET LibKiteConnect)

set(SOURCES
    Kite.cpp
    KiteTypes.h
    KiteTypes.cpp
    Constants.h
    Constants.cpp
    Utils.h
    Utils.cpp
)

# Declare target lib
add_library(${TARGET} SHARED ${SOURCES})

#Extra flags
# Make linking fail if there are any undefined symbols
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
#set(CMAKE_CXX_FLAGS           " ${CMAKE_CXX_FLAGS_INIT} -fPIC -std=c++17") # Make it a library for creating another lib
set(CMAKE_CXX_FLAGS           " ${CMAKE_CXX_FLAGS_INIT} -std=c++17") # Make it a library for creating another lib

target_link_libraries(cpprest)

target_include_directories(${TARGET} PRIVATE .)
