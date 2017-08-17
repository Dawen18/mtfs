include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../../mtFS/include ../../Plugin/)

set(BLOCK_SRCS ../../Plugin/BlockDevice/BlockDevice.h ../../Plugin/BlockDevice/BlockDevice.cpp)

add_definitions(-DDEBUG)
add_executable(blockTests blockDeviceTests.cpp ${BLOCK_SRCS} ../../mtFS/src/utils/Logger.cpp)

target_link_libraries(blockTests gtest gtest_main boost_system)

set(CMAKE_PREFIX_PATH /opt/aws-build)

find_package(aws-sdk-cpp)
add_definitions(-DUSE_IMPORT_EXPORT)
set(S3_SRCS ../../Plugin/S3/S3.h ../../Plugin/S3/S3.cpp ../../mtFS/src/utils/Logger.cpp)
add_executable(s3Tests s3Tests.cpp ${S3_SRCS})

target_link_libraries(s3Tests gtest gtest_main boost_system aws-cpp-sdk-s3)