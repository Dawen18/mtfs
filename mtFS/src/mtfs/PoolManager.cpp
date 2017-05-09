#include "mtfs/PoolManager.h"

namespace mtfs {


	int PoolManager::addPool(uint32_t poolId, Pool *pool, Rule *rule) {
		if (pools.find(poolId) != pools.end())
			return POOL_ID_EXIST;

		pools[poolId] = pool;
		rules[poolId] = rule;

		return SUCCESS;
	}

	bool PoolManager::addBlock(inode_st &inode) {
		return false;
	}

	bool PoolManager::delBlock(inode_st &inode) {
		return false;
	}

	bool PoolManager::getBlock(inode_st &inode, int blockNumber, uint8_t *buffer) {
		return false;
	}

	bool PoolManager::setBlock(inode_st &inode, int blockNb, uint8_t *buffer) {
		return false;
	}

	bool PoolManager::addEntry(inode_st &parentInode, std::string entry, inode_st &entryInode) {
		return false;
	}

	bool PoolManager::delEntry(inode_st &parentInode, std::string entry) {
		return false;
	}

	bool PoolManager::getEntry(inode_st &parentInode, std::string entry, inode_st &entryInode) {
		return false;
	}

	bool PoolManager::setEntry(inode_st &parentInode, std::string entry, inode_st &entryInode) {
		return false;
	}

	bool PoolManager::addLink(inode_st &parentInode, std::string link, ident_st linkId) {
		return false;
	}

	bool PoolManager::delLink(inode_st &parentInode, std::string link) {
		return false;
	}

	bool PoolManager::getRoot(inode_st &rootInode) {
		return false;
	}

	bool PoolManager::addInode(inode_st &inode) {
		return false;
	}

	bool PoolManager::delInode(ident_st inodeId) {
		return false;
	}

	bool PoolManager::getInode(ident_st inodeId, inode_st &inode) {
		return false;
	}

	bool PoolManager::setInode(ident_st inodeId, inode_st &inode) {
		return false;
	}


	ident_t PoolManager::addBlockToEnd(inode_t inode) {
		ident_t ident;
		return ident;
	}

	bool PoolManager::freeLastBlock(inode_t inode) {
		return false;
	}

	bool PoolManager::choosePool(ruleInfo_t info, Pool *pool) {
		return false;
	}

}  // namespace mtfs
