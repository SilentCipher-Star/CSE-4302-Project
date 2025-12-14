cmake_minimum_required(VERSION 3.10)
project(Acadence)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define source files
file(GLOB SOURCES "src/*.cpp")

# Create the executable
add_executable(Acadence ${SOURCES})