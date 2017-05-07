#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "FileStorage/PoolManager.h"

namespace FileStorage {

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
}  // namespace FileStorage
