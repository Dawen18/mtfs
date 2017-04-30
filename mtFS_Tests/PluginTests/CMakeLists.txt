include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../../mtFS/include ../../Plugin/)

set(BLOCK_SRCS ../../Plugin/BlockDevice/BlockDevice.h ../../Plugin/BlockDevice/BlockDevice.cpp)
add_definitions(-DDEBUG)
add_executable(blockTests blockDeviceTests.cpp ${BLOCK_SRCS})

target_link_libraries(blockTests gtest gtest_main)