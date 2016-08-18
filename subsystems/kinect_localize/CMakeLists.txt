cmake_minimum_required (VERSION 2.8)
project(Sound_Localize_Kinect)

# Require C++11
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

include_directories(include)
include_directories(/usr/include/glib-2.0)
include_directories(/usr/lib/x86_64-linux-gnu/glib-2.0/include)

# Add freenect libraries and required fix for Kinect Model #1473
set(freenect_fix_SRCS src/ofxKinectExtras.cpp)
add_library(freenect_fix ${freenect_fix_SRCS})

# Add sources required for dsp
set(dsp_SRCS src/xcor_td.cpp src/Kinect_DOA.cpp)
add_library(dsp ${dsp_SRCS})

# Add sources for JSON
set(json_SRCS include/superstar/jsoncpp/json_value.cpp include/superstar/jsoncpp/json_reader.cpp include/superstar/jsoncpp/json_writer.cpp)
add_library(json ${json_SRCS})

# Add utils
set(utils_SRCS include/superstar/mongoose/mongoose.c include/superstar/string_util.cpp include/superstar/auth.cpp include/superstar/json_util.cpp include/superstar/wget.cpp src/robot_config.cpp src/ini.cpp src/string_util.cpp)
add_library(utils ${utils_SRCS})

# Add superstar
set(superstar_SRCS include/superstar/superstar.cpp)
add_library(superstar ${superstar_SRCS})

# Need to link these before-hand or it will not work.
target_link_libraries(utils json)
target_link_libraries(superstar utils)

# Build the executable
add_executable(Sound_Localize_Kinect Sound_Localize_Kinect.cpp)
# Link against required libraries
target_link_libraries(Sound_Localize_Kinect freenect freenect_fix dsp json utils superstar pthread glib-2.0 gobject-2.0 skeltrack-0.1)

