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

# ============ PLATFORM-SPECIFIC RAYLIB LINKING ============

if(WIN32)
    # Windows: Use RAYLIB_PATH (e.g., C:/raylib/raylib from tasks.json)
    if(NOT DEFINED RAYLIB_PATH)
        set(RAYLIB_PATH "C:/raylib/raylib")
    endif()
    
    target_include_directories(Acadence PRIVATE ${RAYLIB_PATH}/src)
    link_directories(${RAYLIB_PATH}/src)
    target_link_libraries(Acadence PRIVATE raylib opengl32 gdi32 winmm)
    
    message(STATUS "Windows build: Using RAYLIB_PATH=${RAYLIB_PATH}")
    
else()
    # Linux/CachyOS: Prefer system-installed raylib via pkg-config
    find_package(PkgConfig QUIET)
    
    if(PkgConfig_FOUND)
        pkg_check_modules(RAYLIB QUIET raylib)
    endif()
    
    if(RAYLIB_FOUND)
        # System raylib found (e.g. via pacman -S raylib)
        target_include_directories(Acadence PRIVATE ${RAYLIB_INCLUDE_DIRS})
        target_link_libraries(Acadence PRIVATE ${RAYLIB_LIBRARIES})
        target_compile_options(Acadence PRIVATE ${RAYLIB_CFLAGS_OTHER})
        message(STATUS "Linux/CachyOS: Using system raylib ${RAYLIB_VERSION}")
    else()
        # Fallback: look for raylib in standard locations
        find_path(RAYLIB_INCLUDE_DIR raylib.h)
        find_library(RAYLIB_LIBRARY NAMES raylib)
        
        if(RAYLIB_INCLUDE_DIR AND RAYLIB_LIBRARY)
            target_include_directories(Acadence PRIVATE ${RAYLIB_INCLUDE_DIR})
            target_link_libraries(Acadence PRIVATE ${RAYLIB_LIBRARY} GL m pthread dl rt X11)
            message(STATUS "Linux/CachyOS: Found raylib at ${RAYLIB_LIBRARY}")
        else()
            message(FATAL_ERROR "raylib not found on Linux. Install with: sudo dnf install raylib-devel (Fedora) or sudo pacman -S raylib (Arch)")
        endif()
    endif()
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Project: ${PROJECT_NAME}")