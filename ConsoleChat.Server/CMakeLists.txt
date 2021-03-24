cmake_minimum_required(VERSION 3.10)

project(ConsoleChat.Server)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

find_package(Boost 1.75.0 REQUIRED COMPONENTS system filesystem thread)

add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)

include_directories(${Boost_INCLUDE_DIRS} src ../ConsoleChat.Common/src)

link_directories(${Boost_LIBRARY_DIRS})

add_executable(ConsoleChat.Server main.cpp src/ConnectionHandler.cpp src/Connection.cpp src/MessageRouter.cpp)
target_link_libraries(ConsoleChat.Server ${Boost_LIBRARIES})
