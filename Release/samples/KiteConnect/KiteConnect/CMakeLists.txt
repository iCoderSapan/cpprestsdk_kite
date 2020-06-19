project(LibKiteConnect)

# Project targets
set(TARGET LibKiteConnect)

set(SOURCES
    Kite.cpp
    KiteTypes.h
    Utils.h
    Utils.cpp
)

# Declare target lib
add_library(${TARGET} ${SOURCES})

#Extra flags
# Make linking fail if there are any undefined symbols
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
set(CMAKE_CXX_FLAGS           " ${CMAKE_CXX_FLAGS_INIT} -fPIC") # Make it a library for creating another lib

target_include_directories(${TARGET} PRIVATE .)
