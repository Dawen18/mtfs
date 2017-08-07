add_subdirectory(BlockDevice)
add_subdirectory(S3)

set(PLUGIN_H ../mtFS/include/pluginSystem/Plugin.h)

#set(BLOCK_SRCS BlockDevice/BlockDevice.h BlockDevice/BlockDevice.cpp ${PLUGIN_H})
#add_executable(Block ${BLOCK_SRCS})
#
#set(S3_SRCS S3/S3.h S3/S3.cpp ${PLUGIN_H})
#add_executable(S3exe ${S3_SRCS})
#target_link_libraries(S3exe aws-cpp-sdk-s3)