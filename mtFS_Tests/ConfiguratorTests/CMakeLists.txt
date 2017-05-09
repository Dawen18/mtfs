include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../../mtFS/include ../../Plugin/)

set(SRCS
		../../Plugin/BlockDevice/BlockDevice.cpp
		../../mtFS/src/mtfs/Mtfs.cpp
		)

add_definitions(-DDEBUG)
add_executable(configFileTests configFileTests.cpp ${SRCS})

target_link_libraries(configFileTests gtest gtest_main)