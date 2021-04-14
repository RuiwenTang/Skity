
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

add_executable(math_test math_test.cc)
target_link_libraries(math_test gtest skity)

add_executable(geometry_test geometry_test.cc)
target_link_libraries(geometry_test gtest skity)

add_executable(path_test path_test.cc)
target_link_libraries(path_test gtest skity)