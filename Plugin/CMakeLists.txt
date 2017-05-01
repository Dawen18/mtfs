set(PLUGIN_H ../mtFS/include/PluginSystem/Plugin.h)

set(BLOCK_SRCS BlockDevice/BlockDevice.h BlockDevice/BlockDevice.cpp ${PLUGIN_H})
add_executable(Block ${BLOCK_SRCS})