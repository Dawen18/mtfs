include_directories(include)

set(FS_HEAD
		include/FileStorage/structs.h
		)

set(MTFUSE_HEAD
		include/MtfsFuse/FuseBase.h
		include/MtfsFuse/FuseCallback.h
		)

set(MTFUSE_SRC
		src/MtfsFuse/FuseBase.cpp
		src/MtfsFuse/FuseCallback.cpp
		)

add_executable(mtFS ${MTFUSE_SRC} ${MTFUSE_HEAD} ${FS_HEAD} src/main.cpp)
target_link_libraries(mtFS fuse3)