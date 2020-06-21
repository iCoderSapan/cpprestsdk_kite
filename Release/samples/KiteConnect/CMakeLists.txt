project(KiteConnectApp)

# Project targets
set(TARGET KiteConnectApp)

set(SOURCES
    logging.cpp
    logging.h
    Kite.cpp
    KiteTypes.h
    KiteTypes.cpp
    Constants.h
    Constants.cpp
    Utils.h
    Utils.cpp
    main.cpp
)

set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS_INIT} -std=c++17") # Make it a library for creating another lib

target_link_libraries(cpprest)

###########################################################

if (NOT WINDOWS_STORE AND NOT WINDOWS_PHONE)
    add_executable(KiteConnectApp ${SOURCES})
    target_link_libraries(KiteConnectApp cpprest)
endif()
