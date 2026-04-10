cmake_minimum_required(VERSION 3.15)
project(QuickBuild CXX)
# Require C++17 or higher
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Define the executable
add_executable(ireporter ireporter.cpp)