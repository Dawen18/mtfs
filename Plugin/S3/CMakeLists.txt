project(S3)

set(CMAKE_BUILD_TYPE Release)

add_library(s3 SHARED S3.cpp ../../mtFS/src/utils/Logger.cpp)
target_link_libraries(s3 aws-cpp-sdk-s3)
