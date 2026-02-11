cmake_minimum_required(VERSION 3.10)
project(Acadence CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect all source files
file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

# Create executable
add_executable(Acadence ${SOURCES})

# Include directories for project headers
target_include_directories(Acadence PRIVATE 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src
)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Project: ${PROJECT_NAME}")