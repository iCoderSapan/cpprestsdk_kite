cmake_minimum_required(VERSION 3.9)

project(KiteConnect)

# Project targets
set(TARGET KiteConnectApp)

#Extra flags
# Make linking fail if there are any undefined symbols
set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS_INIT} -std=c++17") # Make it a library for creating another lib

if (NOT WINDOWS_STORE AND NOT WINDOWS_PHONE)
    add_subdirectory(KiteConnect)
    add_executable(KiteConnectApp main.cpp)
    target_link_libraries(KiteConnectApp LibKiteConnect)# cpprest)
endif()
