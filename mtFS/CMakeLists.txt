
set(FS_HEAD
		include/mtfs/BlockAccess.h
		include/mtfs/BlockCache.h
		include/mtfs/Cache.h
		include/mtfs/DirectoryEntryAccess.h
		include/mtfs/DirectoryEntryCache.h
		include/mtfs/InodeAcces.h
		include/mtfs/InodeCache.h
		include/mtfs/Mtfs.h
		include/mtfs/Pool.h
		include/mtfs/PoolManager.h
		include/mtfs/Rule.h
		include/mtfs/structs.h
		include/mtfs/TimeRule.h
		include/mtfs/UserRightRule.h
		include/mtfs/Volume.h
		)

set(FS_SRC
		src/mtfs/Cache.cpp
		src/mtfs/Mtfs.cpp
		src/mtfs/Pool.cpp
		src/mtfs/PoolManager.cpp
		src/mtfs/Rule.cpp
		src/mtfs/TimeRule.cpp
		src/mtfs/UserRightRule.cpp
		src/mtfs/Volume.cpp)

set(MTFUSE_HEAD
		include/MtfsFuse/FuseBase.h
		include/MtfsFuse/FuseCallback.h
		include/MtfsFuse/MtfsFuse.h
		)

set(MTFUSE_SRC
		src/MtfsFuse/FuseBase.cpp
		src/MtfsFuse/FuseCallback.cpp
		src/MtfsFuse/MtfsFuse.cpp
		)

set(CONF_HEAD
		include/pluginSystem/PluginManager.h
		)

set(CONF_SRC
		src/pluginSystem/PluginManager.cpp
		)

add_executable(mtfsCreate ${MTFUSE_SRC} ${MTFUSE_HEAD} ${FS_HEAD} ${FS_SRC} src/main.cpp)
target_link_libraries(mtfsCreate fuse3 pthread)

add_definitions(-DDEBUG)
add_executable(mtfsMount src/mtfsMount.cpp ${CONF_HEAD} ${CONF_SRC} ${FS_HEAD} ${FS_SRC} ../Plugin/BlockDevice/BlockDevice.cpp)
target_link_libraries(mtfsMount fuse3 pthread)
