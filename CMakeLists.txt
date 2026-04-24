cmake_minimum_required(VERSION 3.15)
project(QuickBuild CXX)
# Require C++17 or higher
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Define include directories
include_directories(. CLI)

# Define the shared library
add_library(iprovider SHARED iprovider.cpp)
target_compile_definitions(iprovider PRIVATE UNMANGLED_LIB_EXPORTS)

# Define the executable
add_executable(ireporter main.cpp ireporter.cpp)
# No target_link_libraries(ireporter PRIVATE iprovider) as we load it dynamically
