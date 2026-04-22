cmake_minimum_required(VERSION 3.16)

project(TradeEngine LANGUAGES CXX)

#Use Modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Create Executable
add_executable(TradeEngine
    src/Main.cpp
    src/TradeProcessor.cpp
    src/TradeStrategy.cpp
    src/Logger.cpp
)

#Find Header files
target_include_directories(TradeEngine
    PUBLIC
       ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
