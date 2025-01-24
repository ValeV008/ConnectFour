cmake_minimum_required(VERSION 3.10)

project(ConnectFour)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(jsoncpp)

# Find Boost
if (WIN32)
    set(Boost_USE_STATIC_LIBS ON)
    if ("${BOOST_ROOT}" STREQUAL "")
        set(BOOST_ROOT "C:/boost_1_87_0")
    endif()
endif()
find_package(Boost REQUIRED COMPONENTS random)

# Add DatabaseManager library
add_library(DatabaseManager STATIC
    DatabaseManager.cpp
    DatabaseManager.h
    sqlite3/sqlite3.c
)
target_include_directories(DatabaseManager PUBLIC ${CMAKE_SOURCE_DIR}/sqlite3)

# Add ConnectFourGame library
add_library(ConnectFourGame STATIC
    ConnectFourGame.cpp
    ConnectFourGame.h
)
target_link_libraries(ConnectFourGame PUBLIC DatabaseManager jsoncpp_static)


# Add server executable
add_executable(server server.cpp)
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/websocketpp)
target_link_libraries(server
    Boost::random
    ConnectFourGame
    DatabaseManager
)


# Add client executable
add_executable(client client.cpp)
target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR}/websocketpp)
target_link_libraries(client
    Boost::random
    jsoncpp_static
)


# Add random_luka executable
add_executable(random_luka
    RandomLukaBot.cpp
    Bot.cpp
)
target_include_directories(random_luka PRIVATE ${CMAKE_SOURCE_DIR}/websocketpp)
target_link_libraries(random_luka
    Boost::random
    jsoncpp_static
)


# Add random_janez executable
add_executable(random_janez
    RandomJanezBot.cpp
    Bot.cpp
)
target_include_directories(random_janez PRIVATE ${CMAKE_SOURCE_DIR}/websocketpp)
target_link_libraries(random_janez
    Boost::random
    jsoncpp_static
)