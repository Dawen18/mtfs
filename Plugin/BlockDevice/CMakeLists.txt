project(blockDevice)

set(CMAKE_BUILD_TYPE Release)

add_library(block SHARED BlockDevice.cpp ../../mtFS/src/utils/Logger.cpp)
