cmake_minimum_required(VERSION 3.10)

project(ConsoleChat.Client)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

find_package(Boost 1.75.0 REQUIRED COMPONENTS system filesystem thread)

include_directories(${Boost_INCLUDE_DIRS} ../ConsoleChat.Common/src src)
link_directories(${Boost_LIBRARY_DIRS})

add_executable(ConsoleChat.Client main.cpp src/MessageExchanger.cpp)
target_link_libraries(ConsoleChat.Client ${Boost_LIBRARIES})
