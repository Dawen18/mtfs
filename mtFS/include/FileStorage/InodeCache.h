#ifndef FILESTORAGE_INODE_CACHE_H
#define FILESTORAGE_INODE_CACHE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>
#include <FileStorage/structs.h>

namespace FileStorage {
	class InodeCache {
	public:
		virtual bool replaceInode(ident_st inodeId, inode_st &inode)=0;

	};

}  // namespace FileStorage
#endif
