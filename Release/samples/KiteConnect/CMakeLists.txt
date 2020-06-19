project(KiteConnect)

# Project targets
set(TARGET KiteConnectApp)


if (NOT WINDOWS_STORE AND NOT WINDOWS_PHONE)
    add_subdirectory(KiteConnect)
    add_executable(KiteConnectApp main.cpp)
    target_link_libraries(KiteConnectApp cpprest)
endif()
