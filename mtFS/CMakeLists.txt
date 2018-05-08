if (NOT DEFINED MTFS_HOME_DIR)
	set(MTFS_HOME_DIR /home/mtfs)
endif ()
add_definitions(-DMTFS_HOME_DIR="${MTFS_HOME_DIR}")
add_definitions(-DMTFS_PLUGIN_LIB="${MTFS_HOME_DIR}/Libs/")
add_definitions(-DMTFS_PLUGIN_HOME="${MTFS_HOME_DIR}/Plugins/")
add_definitions(-DMTFS_CONFIG_DIR="${MTFS_HOME_DIR}/Configs/")
add_definitions(-DMTFS_INSTALL_DIR="${MTFS_HOME_DIR}/Systems/")

set(FS_HEAD
		include/mtfs/Cache.h
		include/mtfs/Acces.h
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
		include/wrapper/FuseBase.h
		include/wrapper/FuseCallback.h
		include/wrapper/MtfsFuse.h
		)

set(MTFUSE_SRC
		src/wrapper/FuseBase.cpp
		src/wrapper/FuseCallback.cpp
		src/wrapper/MtfsFuse.cpp
		)

set(CONF_HEAD
		include/pluginSystem/PluginManager.h
		include/pluginSystem/Plugin.h
		src/utils/Semaphore.cpp include/utils/Semaphore.h src/mtfs/Migrator.cpp include/mtfs/Migrator.h)

set(CONF_SRC
		src/pluginSystem/PluginManager.cpp
		)

set(UTILS
		include/utils/Fs.h
		src/utils/Fs.cpp
		src/utils/Semaphore.cpp
		include/utils/Semaphore.h
		src/utils/Logger.cpp
		include/utils/Logger.h
		)

set(CMAKE_PREFIX_PATH /opt/aws-build)
#find_package(aws-sdk-cpp)
add_definitions(-DUSE_IMPORT_EXPORT)

add_executable(mtfsCreate src/mtfsCreate.cpp
		${FS_SRC} ${CONF_SRC} ${UTILS})

target_link_libraries(mtfsCreate fuse3 dl boost_system boost_filesystem boost_thread pthread)


#add_definitions(-DDEBUG)

add_executable(mtfsMount src/mtfsMount.cpp
		${CONF_HEAD} ${CONF_SRC}
		${FS_HEAD} ${FS_SRC}
		${MTFUSE_HEAD} ${MTFUSE_SRC}
		${UTILS}
		)

target_link_libraries(mtfsMount fuse3 dl boost_system boost_filesystem boost_thread pthread)
