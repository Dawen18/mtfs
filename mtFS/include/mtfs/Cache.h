#ifndef FILESTORAGE_CACHE_H
#define FILESTORAGE_CACHE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "mtfs/DirectoryEntryCache.h"
#include "mtfs/Acces.h"
#include "mtfs/InodeCache.h"
#include "mtfs/BlockCache.h"
#include "mtfs/PoolManager.h"

namespace mtfs {
	class Cache :
			public InodeCache,
			public Acces,
			public BlockCache,
			public DirectoryEntryCache {
	private:
		std::map<ident_t, inode_t> inodeCache;

		std::map<ident_t, uint8_t *> directoryCache;

		std::map<ident_t, uint8_t *> blockCache;

		PoolManager *storage;

	};

}  // namespace mtfs
#endif
