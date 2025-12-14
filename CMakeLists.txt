cmake_minimum_required(VERSION 3.10)
project(Acadence)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define Raylib path (matches your tasks.json configuration)
set(RAYLIB_PATH "C:/raylib/raylib")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${RAYLIB_PATH}/src)
include_directories(${PROJECT_SOURCE_DIR})

# Define source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create the executable
add_executable(Acadence ${SOURCES})

# Link Raylib and system libraries
link_directories(${RAYLIB_PATH}/src)
if(WIN32)
    target_link_libraries(Acadence raylib opengl32 gdi32 winmm)
else()
    target_link_libraries(Acadence raylib GL m pthread dl rt X11)
endif()