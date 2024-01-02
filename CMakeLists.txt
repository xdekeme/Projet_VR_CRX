# Heavily inspired from https://github.com/Polytonic/Glitter/blob/master/CMakeLists.txt

# Project setup
project(project_crx)
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Define some variables
set(COMPILE_PROJECT_CRX ON CACHE BOOL "Compile the VR project")

find_package(OpenGL REQUIRED)

# Include directories for GLAD, GLFW, GLM, STB
include_directories(/usr/local/include)
include_directories(3rdParty/glad/include/
                    3rdParty/glm/
                    3rdParty/stb/
                    3rdParty/tinyxml/
                    3rdParty/bullet3-master/src/
                    3rdParty/assimp-master/)

# For glad library
add_library(glad STATIC 3rdParty/glad/src/glad.c)
set(GLAD_INCLUDE "3rdParty/glad/include")
target_include_directories(glad PUBLIC ${GLAD_INCLUDE})

# For GLFW
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
find_package(glfw3 QUIET)
if (NOT glfw3_FOUND)
    add_subdirectory(3rdParty/glfw)
    include_directories(3rdParty/glfw/include/)
endif()

# Add the subdirectory containing the project
if(COMPILE_PROJECT_CRX)
    add_subdirectory(PROJECT_CRX)
endif()

# TinyXML sources
set(TINYXML_SOURCES 
    3rdParty/tinyxml/tinyxml.cpp 
    3rdParty/tinyxml/tinystr.cpp
    3rdParty/tinyxml/tinyxmlerror.cpp 
    3rdParty/tinyxml/tinyxmlparser.cpp)

# Add TinyXML library
add_library(tinyxml STATIC ${TINYXML_SOURCES})
target_include_directories(tinyxml PUBLIC 3rdParty/tinyxml/)

#Bullet3-master library
add_subdirectory(3rdParty/bullet3-master)

#Assimp-master library
add_subdirectory(3rdParty/assimp-master)
