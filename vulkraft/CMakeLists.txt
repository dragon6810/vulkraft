# Minimum CMake version required
cmake_minimum_required(VERSION 3.14)

# Project name and version
project(vulkraft VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Output directories for build files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find Vulkan
find_package(Vulkan REQUIRED)

# Find GLFW
find_package(GLFW3 REQUIRED)

# Find GLM (header-only library)
find_path(GLM_INCLUDE_DIR glm/glm.hpp)

# Include directories
include_directories(${Vulkan_INCLUDE_DIRS})
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIR})
include_directories(src/)

# Source files
file(GLOB_RECURSE SOURCES src/*.cpp)

# Add executable
add_executable(vulkraft ${SOURCES})

# Link libraries
target_link_libraries(vulkraft PRIVATE Vulkan::Vulkan glfw)

# Add custom settings for macOS and Linux
if(APPLE)
    message(STATUS "Building for macOS")
    target_compile_definitions(vulkraft PRIVATE MACOS)
elseif(UNIX AND NOT APPLE)
    message(STATUS "Building for Linux")
    target_compile_definitions(vulkraft PRIVATE LINUX)
endif()