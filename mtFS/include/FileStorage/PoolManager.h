#ifndef FILESTORAGE_POOL_MANAGER_H
#define FILESTORAGE_POOL_MANAGER_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "FileStorage/DirectoryEntryAccess.h"
#include "FileStorage/BlockAccess.h"
#include "FileStorage/InodeAcces.h"
#include "FileStorage/Pool.h"
#include "FileStorage/InodeCache.h"
#include "FileStorage/BlockCache.h"
#include "FileStorage/DirectoryEntryCache.h"

namespace FileStorage {
	class PoolManager : public DirectoryEntryAccess, public BlockAccess, public InodeAcces {
	private:
		std::map<uint32_t, Pool> pools;

		InodeCache *inodeCache;

		BlockCache *blocksCache;

		DirectoryEntryCache *dirEntryCache;

		std::vector<ident_t> lockedBlocks;

		std::vector<ident_t> lockedInodes;

		std::map<ident_t, ident_t> translateMap;


	private:
		ident_t addBlockToEnd(inode_st inode);

		bool freeLastBlock(inode_st inode);

		bool choosePool(ruleInfo_st info, Pool *pool);

	};

}  // namespace FileStorage
#endif
