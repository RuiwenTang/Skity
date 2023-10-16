

cmake_minimum_required(VERSION 3.13)

add_subdirectory(../../../../../../ ../../../../../../../build)
set(SRC GLDemoJNI.cpp ../../../../../../example/example.cc)
add_library(GLDemoJni SHARED ${SRC})



find_library(
        gles-lib
        GLESv3
)
find_library(
        EGL-lib
        EGL
)
target_link_libraries(GLDemoJni PRIVATE ${gles-lib} ${EGL-lib} skity)

