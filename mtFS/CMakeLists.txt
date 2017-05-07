
set(FS_HEAD
		include/FileStorage/BlockAccess.h
		include/FileStorage/BlockCache.h
		include/FileStorage/Cache.h
		include/FileStorage/DirectoryEntryAccess.h
		include/FileStorage/DirectoryEntryCache.h
		include/FileStorage/InodeAcces.h
		include/FileStorage/InodeCache.h
		include/FileStorage/Mtfs.h
		include/FileStorage/Pool.h
		include/FileStorage/PoolManager.h
		include/FileStorage/Rule.h
		include/FileStorage/structs.h
		include/FileStorage/TimeRule.h
		include/FileStorage/UserRightRule.h
		include/FileStorage/Volume.h
		)

set(FS_SRC
		src/FileStorage/Cache.cpp
		src/FileStorage/Mtfs.cpp
		src/FileStorage/Pool.cpp
		src/FileStorage/PoolManager.cpp
		src/FileStorage/TimeRule.cpp
		src/FileStorage/UserRightRule.cpp
		src/FileStorage/Volume.cpp)

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
		include/Configurator/ConfigFile.h
		)

set(CONF_SRC
		src/Configurator/ConfigFile.cpp)

add_executable(mtfsCreate ${MTFUSE_SRC} ${MTFUSE_HEAD} ${FS_HEAD} ${FS_SRC} src/main.cpp)
target_link_libraries(mtfsCreate fuse3 pthread)

add_definitions(-DDEBUG)
add_executable(mtfsMount src/mtfsMount.cpp ${CONF_HEAD} ${CONF_SRC} ${FS_HEAD} ${FS_SRC} ../Plugin/BlockDevice/BlockDevice.cpp)
target_link_libraries(mtfsMount fuse3 pthread)
