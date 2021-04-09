
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

add_executable(math_test math_test.cc)
target_link_libraries(math_test gtest skity)