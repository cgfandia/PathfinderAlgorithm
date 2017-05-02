cmake_minimum_required(VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)
# Project Name
PROJECT(PATHFINDER)
#########################################################
# FIND OPENMP
#########################################################
FIND_PACKAGE( OpenMP REQUIRED)
if(OPENMP_FOUND)
message("OPENMP FOUND")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
#########################################################
# FIND THREADS
#########################################################
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
#########################################################
# FIND GLUT
#########################################################
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})
add_definitions(${GLUT_DEFINITIONS})
if(NOT GLUT_FOUND)
    message(ERROR " freeglut3 and freeglut3-dev not found!")
endif(NOT GLUT_FOUND)
#########################################################
# FIND OPENGL
#########################################################
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)
#########################################################
# Include Files
#########################################################
set(srcDir src/)
#add_library(FPGA STATIC ${srcDir}FPGA.cpp)
#add_library(pathfinderLib STATIC ${srcDir}pathfinder.cpp)
add_library(viewer STATIC ${srcDir}viewer.cpp)
add_library(pathfinderLib STATIC ${srcDir}pathfinder.cpp ${srcDir}FPGA.cpp)
add_executable(${PROJECT_NAME} ${srcDir}main.cpp)

########################################################
# Linking & stuff
#########################################################
target_link_libraries(${PROJECT_NAME} pathfinderLib viewer ${CMAKE_THREAD_LIBS_INIT} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
