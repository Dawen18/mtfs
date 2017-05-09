include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(../../mtFS/include ../../Plugin/)

set(SRCS
		../../Plugin/BlockDevice/BlockDevice.cpp
		../../mtFS/src/mtfs/Mtfs.cpp
		../../mtFS/src/mtfs/TimeRule.cpp
		../../mtFS/src/mtfs/PoolManager.cpp
		../../mtFS/src/mtfs/Volume.cpp
		../../mtFS/src/mtfs/Pool.cpp
		../../mtFS/src/mtfs/Rule.cpp
		../../mtFS/src/mtfs/UserRightRule.cpp
		../../mtFS/src/pluginSystem/PluginManager.cpp
		)

add_definitions(-DDEBUG)
add_executable(configFileTests configFileTests.cpp ${SRCS})

target_link_libraries(configFileTests gtest gtest_main)