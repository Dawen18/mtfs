
set(FS_HEAD
		include/mtfs/BlockCache.h
		include/mtfs/Cache.h
		include/mtfs/DirectoryEntryCache.h
		include/mtfs/Acces.h
		include/mtfs/InodeCache.h
		include/mtfs/Mtfs.h
		include/mtfs/Pool.h
		include/mtfs/PoolManager.h
		include/mtfs/Rule.h
		include/mtfs/structs.h
		include/mtfs/TimeRule.h
		include/mtfs/UserRightRule.h
		include/mtfs/Volume.h
		include/mtfs/Migrator.h)

set(FS_SRC
		src/mtfs/Cache.cpp
		src/mtfs/Mtfs.cpp
		src/mtfs/Pool.cpp
		src/mtfs/PoolManager.cpp
		src/mtfs/Rule.cpp
		src/mtfs/TimeRule.cpp
		src/mtfs/UserRightRule.cpp
		src/mtfs/Volume.cpp
		src/mtfs/Migrator.cpp)

set(MTFUSE_HEAD
		include/mtfsFuse/FuseBase.h
		include/mtfsFuse/FuseCallback.h
		include/mtfsFuse/MtfsFuse.h
		)

set(MTFUSE_SRC
		src/mtfsFuse/FuseBase.cpp
		src/mtfsFuse/FuseCallback.cpp
		src/mtfsFuse/MtfsFuse.cpp
		)

set(CONF_HEAD
		include/pluginSystem/PluginManager.h
		include/pluginSystem/Plugin.h
		src/utils/Semaphore.cpp include/utils/Semaphore.h src/mtfs/Migrator.cpp include/mtfs/Migrator.h)

set(CONF_SRC
		src/pluginSystem/PluginManager.cpp
		)

set(UTILS
		include/utils/ThreadQueue.h
		include/utils/Fs.h
		src/utils/Fs.cpp
		src/utils/Semaphore.cpp
		include/utils/Semaphore.h
		src/utils/Logger.cpp
		include/utils/Logger.h
		)

set(CMAKE_PREFIX_PATH /opt/aws-build)
find_package(aws-sdk-cpp)
add_definitions(-DUSE_IMPORT_EXPORT)

add_executable(mtfsCreate src/mtfsCreate.cpp
		${FS_SRC} ${CONF_SRC} ${UTILS} ../Plugin/S3/S3.cpp)

target_link_libraries(mtfsCreate fuse3 dl boost_system boost_filesystem boost_thread pthread aws-cpp-sdk-s3)

#link_directories(/usr/local/lib)

#add_definitions(-DDEBUG)
add_executable(mtfsMount src/mtfsMount.cpp
		${CONF_HEAD} ${CONF_SRC}
		${FS_HEAD} ${FS_SRC}
		${MTFUSE_HEAD} ${MTFUSE_SRC}
		${UTILS}
		../Plugin/S3/S3.cpp
		)

target_link_libraries(mtfsMount fuse3 dl boost_system boost_filesystem boost_thread pthread aws-cpp-sdk-s3)
