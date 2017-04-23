include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../../mtFS/include)

add_executable(test_tt test.cpp)

target_link_libraries(test_tt gtest gtest_main)