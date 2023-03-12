

cmake_minimum_required(VERSION 3.13)

#add_subdirectory(../../../../../../CMakeLists.txt)
add_subdirectory(../../../../../../ ../../../../../../../build)
include_directories(../../../../../../include)
set(SRC GLDemoJNI.cpp ../../../../../../example/example.cc)
add_library(GLDemoJni SHARED ${SRC})

target_link_libraries(GLDemoJni skity)

