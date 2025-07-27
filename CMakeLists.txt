cmake_minimum_required(VERSION 3.10)
project(InteractiveSolarSystem)

set(CMAKE_CXX_STANDARD 11)

# Include subdirectories if you want to organize by folders
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/Camera)

file(GLOB SOURCES
    main.cpp
    Shader.cpp
    Texture.cpp
    Camera/Camera.cpp
)

add_executable(InteractiveSolarSystem ${SOURCES})

find_library(COCOA_LIBRARY Cocoa)
find_library(OpenGL_LIBRARY OpenGL)
find_library(IOKit_LIBRARY IOKit)
find_library(CoreVideo_LIBRARY CoreVideo)
find_library(GLUT_LIBRARY GLUT)

target_link_libraries(InteractiveSolarSystem
    ${OpenGL_LIBRARY}
    ${GLUT_LIBRARY}
    ${COCOA_LIBRARY}
    ${IOKit_LIBRARY}
    ${CoreVideo_LIBRARY}
    glfw
    GLEW
)
