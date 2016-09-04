#CMake Required...
cmake_minimum_required (VERSION 2.8)

#Flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-result -Wno-sign-compare")
add_definitions(-DMG_ENABLE_SSL)

#Include Directories
set(root_inc ../../include)
include_directories(${root_inc})

set(skeltrack_inc ${root_inc}/skeltrack)
include_directories(${skeltrack_inc})

#Libraries
find_package(PkgConfig)

pkg_search_module(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})

pkg_search_module(OPENSSL REQUIRED openssl)
include_directories(${OPENSSL_INCLUDE_DIR})

#Sources
set(kinect_src src/Kinect_DOA.cpp src/xcor_td.cpp src/ofxKinectExtras.cpp)
add_library(kinect ${kinect_src})

set(skeltrack_src ${skeltrack_inc}/pqueue.c ${skeltrack_inc}/skeltrack-joint.c ${skeltrack_inc}/skeltrack-skeleton.c ${skeltrack_inc}/skeltrack-smooth.c ${skeltrack_inc}/skeltrack-util.c)
add_library(skeltrack ${skeltrack_src})

set(utility_src ${root_inc}/auth.cpp ${root_inc}/ini.cpp ${root_inc}/json_util.cpp ${root_inc}/robot_config.cpp ${root_inc}/string_util.cpp ${root_inc}/superstar.cpp ${root_inc}/time_util.cpp ${root_inc}/wget.cpp)
add_library(utility ${utility_src})

set(jsoncpp_inc ${root_inc}/jsoncpp)
set(jsoncpp_src ${jsoncpp_inc}/json_reader.cpp ${jsoncpp_inc}/json_value.cpp ${jsoncpp_inc}/json_writer.cpp)
add_library(jsoncpp ${jsoncpp_src})

set(mongoose_inc ${root_inc}/mongoose)
set(mongoose_src ${mongoose_inc}/mongoose.c)
add_library(mongoose ${mongoose_src})

add_executable(Sound_Localize_Kinect src/Sound_Localize_Kinect.cpp)

#Final Link
target_link_libraries(Sound_Localize_Kinect ${GLIB2_LIBRARIES} ${OPENSSL_LIBRARIES} kinect skeltrack utility jsoncpp mongoose freenect pthread gio-2.0 gobject-2.0 gthread-2.0 glib-2.0)