
# Set the name and the supported language of the project
project(tsxProject C)
# Set the minimum version of cmake required to build this project
#cmake_minimum_required(VERSION 3.7)
cmake_minimum_required(VERSION 3.0  )

#set( CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_BUILD_TYPE Release) # optimizations, but debuging is not allowed
#set(CMAKE_BUILD_TYPE Debug) # optimizations, but debuging is not allowed

# Use the package PkgConfig to detect GTK+ headers/library files
#find_package(PkgConfig REQUIRED)

# pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
# Setup CMake to use GTK+, tell the compiler where to    look for headers
# and to the linker where to look for libraries
#include_directories(${GTK3_INCLUDE_DIRS})
#link_directories(${GTK3_LIBRARY_DIRS})
# Add other flags to the compiler
#add_definitions(${GTK3_CFLAGS_OTHER})

#include(FindGTK2)
#if (GTK2_FOUND)
#    include_directories(${GTK2_INCLUDE_DIRS})
#    link_directories(${GTK2_LIBRARY_DIRS})
#    link_libraries(${GTK2_LIBRARIES})
#endif (GTK2_FOUND)

set(CMAKE_C_STANDARD 99)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fgnu-tm")

set(SOURCE_FILES main.c TestTSX.c UtilFunctionsTSX.c ThreadUtils.c UtilFunctionsTSX.h ThreadUtils.h TestTSX.h BenchmarkSupport.c BenchmarkSupport.h TestAtomicOps.c TestAtomicOps.h TestCoreAffinity.c TestCoreAffinity.h VectorMatrixMult.c VectorMatrixMult.h Queues.c Queues.h FirstTestHyperThreads.c FirstTestHyperThreads.h)
add_executable(tsxExe ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(tsxExe pthread)
# TARGET_LINK_LIBRARIES(tsxExe capstone) # for lynx queue

TARGET_LINK_LIBRARIES(tsxExe m)