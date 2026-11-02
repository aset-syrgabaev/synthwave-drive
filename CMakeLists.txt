cmake_minimum_required(VERSION 3.16)
project(SynthwaveDrive VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.c
    include/*.h
)

# Executable target
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# OpenGL & Dependencies
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

# System-specific settings (Windows / macOS)
if(WIN32)
    message(STATUS "Configuring for Windows platform")
elseif(APPLE)
    message(STATUS "Configuring for macOS platform")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()
