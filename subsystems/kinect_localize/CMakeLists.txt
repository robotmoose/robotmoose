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

# Add freenect libraries and required fix for Kinect Model #1473
set(freenect_fix_SRCS src/ofxKinectExtras.cpp)
add_library(freenect_fix ${freenect_fix_SRCS})

# Add sources required for dsp
set(dsp_SRCS src/xcor_td.cpp src/Kinect_DOA.cpp)
add_library(dsp ${dsp_SRCS})

# Add sources for JSON
set(json_SRCS include/jsoncpp/json_value.cpp include/jsoncpp/json_reader.cpp include/jsoncpp/json_writer.cpp)
add_library(json ${json_SRCS})

# Add utils
set(utils_SRCS include/mongoose/mongoose.c src/string_util.cpp src/auth.cpp src/json_util.cpp src/wget.cpp)
add_library(utils ${utils_SRCS})

# Add superstar
set(superstar_SRCS src/superstar.cpp) # src/string_util.cpp src/auth.cpp src/json_util.cpp src/wget.cpp)
add_library(superstar ${superstar_SRCS})

# Libraries needed for graphics
set(graphics_SRCS GLEW glfw3 glut Xrandr Xrender Xinerama Xcursor Xi GL m dl drm Xdamage X11-xcb xcb-glx xcb-dri2 xcb-dri3 xcb-present xcb-sync xshmfence Xxf86vm Xfixes Xext X11 pthread xcb Xau Xdmcp)

# Need to link these before-hand or it will not work.
target_link_libraries(utils json)
target_link_libraries(superstar utils)

# Build the executable
add_executable(Sound_Localize_Kinect Sound_Localize_Kinect.cpp)
# Link against required libraries
target_link_libraries(Sound_Localize_Kinect freenect freenect_fix dsp json utils superstar ${graphics_SRCS})

