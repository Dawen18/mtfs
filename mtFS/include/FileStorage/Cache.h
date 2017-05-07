#ifndef FILESTORAGE_CACHE_H
#define FILESTORAGE_CACHE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "FileStorage/DirectoryEntryAccess.h"
#include "FileStorage/InodeCache.h"
#include "FileStorage/InodeAcces.h"
#include "FileStorage/BlockAccess.h"
#include "FileStorage/BlockCache.h"
#include "FileStorage/DirectoryEntryCache.h"
#include "FileStorage/PoolManager.h"

namespace FileStorage {
	class Cache
			: public DirectoryEntryAccess,
			  public InodeCache,
			  public InodeAcces,
			  public BlockAccess,
			  public BlockCache,
			  public DirectoryEntryCache {
	private:
		std::map<ident_t, inode_t> inodeCache;

		std::map<ident_t, uint8_t *> directoryCache;

		std::map<ident_t, uint8_t *> blockCache;

		PoolManager *storage;

	};

}  // namespace FileStorage
#endif
