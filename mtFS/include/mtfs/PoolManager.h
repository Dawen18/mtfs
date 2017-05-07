#ifndef FILESTORAGE_POOL_MANAGER_H
#define FILESTORAGE_POOL_MANAGER_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "mtfs/DirectoryEntryAccess.h"
#include "mtfs/BlockAccess.h"
#include "mtfs/InodeAcces.h"
#include "mtfs/InodeCache.h"
#include "mtfs/BlockCache.h"
#include "mtfs/DirectoryEntryCache.h"
#include "mtfs/Pool.h"

namespace mtfs {
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

	public:
		bool addBlock(inode_st &inode) override;

		bool delBlock(inode_st &inode) override;

		bool getBlock(inode_st &inode, int blockNumber, uint8_t *buffer) override;

		bool setBlock(inode_st &inode, int blockNb, uint8_t *buffer) override;

		bool addEntry(inode_st &parentInode, std::string entry, inode_st &entryInode) override;

		bool delEntry(inode_st &parentInode, std::string entry) override;

		bool getEntry(inode_st &parentInode, std::string entry, inode_st &entryInode) override;

		bool setEntry(inode_st &parentInode, std::string entry, inode_st &entryInode) override;

		bool addLink(inode_st &parentInode, std::string link, ident_st linkId) override;

		bool delLink(inode_st &parentInode, std::string link) override;

		bool getRoot(inode_st &rootInode) override;

		bool addInode(inode_st &inode) override;

		bool delInode(ident_st inodeId) override;

		bool getInode(ident_st inodeId, inode_st &inode) override;

		bool setInode(ident_st inodeId, inode_st &inode) override;

	};

}  // namespace mtfs
#endif
