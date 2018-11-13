cmake_minimum_required(VERSION 2.8)
project( BosonUSB )

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF()

MESSAGE("Build type: " ${CMAKE_BUILD_TYPE})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall   -O3 -march=native ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall    -O3 -march=native")

# Check C++11 or C++0x support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
   add_definitions(-DCOMPILEDWITHC11)
   message(STATUS "Using flag -std=c++11.")
elseif(COMPILER_SUPPORTS_CXX0X)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
   add_definitions(-DCOMPILEDWITHC0X)
   message(STATUS "Using flag -std=c++0x.")
else()
   message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

find_package(Threads REQUIRED)

include_directories( ${PROJECT_SOURCE_DIR}/include 
${TBB_INCLUDE}
${OpenCV_INCLUDE_DIRS})

find_package( OpenCV REQUIRED)

add_library(${PROJECT_NAME} SHARED
src/CameraStreamer.cpp
src/cvMatContainer.cpp
)

target_link_libraries(${PROJECT_NAME}
${OpenCV_LIBS}
${TBB_LIBRARY_RELEASE}
${CMAKE_THREAD_LIBS_INIT}
-ltbb
)

add_executable(test src/main.cpp)
target_link_libraries(test ${PROJECT_NAME} ${OpenCV_LIBS}
${TBB_LIBRARY_RELEASE}
${CMAKE_THREAD_LIBS_INIT}
-ltbb)

