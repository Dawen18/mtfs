#include <mtfs/Mtfs.h>
#include <mtfs/structs.h>
#include "mtfs/PoolManager.h"

namespace mtfs {
	using namespace std;

	PoolManager::~PoolManager() {
		for (auto &&pool: this->pools) {
			delete pool.second;
		}

		for (auto &&rule: this->rules) {
			delete rule.second;
		}
	}

	int PoolManager::addPool(uint32_t poolId, Pool *pool, Rule *rule) {
		if (pools.find(poolId) != pools.end())
			return POOL_ID_EXIST;

		pools[poolId] = pool;
		rules[poolId] = rule;

		return SUCCESS;
	}


	int PoolManager::addInode(const ruleInfo_t &info, std::vector<ident_t> &idents, const int nb) {
		return this->add(info, idents, nb, this->INODE);
	}

	int PoolManager::delInode(const ident_t &inodeId) {
		return false;
	}

	int PoolManager::getInode(const ident_t &inodeId, inode_st &inode) {

		int ret = this->pools[inodeId.poolId]->getInode(inodeId.volumeId, inodeId.id, inode);

		return ret;
	}

	int PoolManager::putInode(const ident_t &inodeId, const inode_t &inode) {
//		if (!this->lock(inodeId, this->INODE))
//			return this->IS_LOCKED;

		ident_t id = inodeId;
//		this->hasMoved(inodeId, id, this->INODE);

		int ret = this->pools[id.poolId]->putInode(id.volumeId, id.id, inode);

		return ret;
	}

	int PoolManager::addDirBlock(const ruleInfo_t &infos, std::vector<ident_t> &blockId, const int nb) {
		return this->add(infos, blockId, nb, this->DIR_BLOCK);
	}

	int PoolManager::delDirBlock(const ident_t &blockId) {
		return false;
	}

	int PoolManager::getDirBlock(const ident_t &blockId, dirBlock_t &block) {
//		if (this->isLocked(blockId, this->DIR_BLOCK))
//			return this->IS_LOCKED;

//		this->lock(blockId, this->DIR_BLOCK);

		int ret = this->pools[blockId.poolId]->getDirBlock(blockId.volumeId, blockId.id, block);

		return ret;
	}

	int PoolManager::putDirBlock(const ident_t &blockId, const dirBlock_t &block) {
//		if (!this->lock(blockId, this->DIR_BLOCK))
//			return this->IS_LOCKED;
//
		ident_t newId = blockId;
//		this->hasMoved(blockId, newId, this->DIR_BLOCK);

		int ret = this->pools[newId.poolId]->putDirBlock(newId.volumeId, newId.id, block);

		return ret;
	}

	int PoolManager::addBlock(const ruleInfo_t &infos, std::vector<ident_t> &ident, const int nb) {
		return this->add(infos, ident, nb, this->BLOCK);
	}

	bool PoolManager::delBlock(inode_st &inode) {
		return false;
	}

	int PoolManager::getBlock(const ident_t &blockId, uint8_t *buffer) {
		return false;
	}

	int PoolManager::putBlock(const ident_t &blockId, uint8_t *buffer) {
//		if (!this->lock(blockId, this->INODE))
//			return this->IS_LOCKED;

		ident_t id = blockId;
//		this->hasMoved(blockId, id, this->INODE);

		int ret = this->pools[id.poolId]->putBlock(id.volumeId, id.id, buffer);

		return ret;
	}


	int PoolManager::add(const ruleInfo_t &info, std::vector<ident_t> &idents, const int nb, const queryType type) {
		int ret;
		vector<uint32_t> poolIds;
		if (0 != (ret = this->getValidPools(info, poolIds))) {
			return ret;
		}
		vector<ident_t> tmpIdent;

		if (nb <= poolIds.size()) {
			for (auto &&id: poolIds) {
				tmpIdent.clear();

				switch (type) {
					case INODE:
						this->pools[id]->addInode(info, tmpIdent);
						break;
					case DIR_BLOCK:
						this->pools[id]->addDirBlock(info, tmpIdent);
						break;
					case BLOCK:
						this->pools[id]->addBlock(info, tmpIdent);
						break;
					default:
//						TODO LOG error
						break;
				}

				for (auto &&ident: tmpIdent) {
					ident.poolId = id;
					idents.push_back(ident);
				}
			}
		} else {
			int blkPerPool = (int) (nb / poolIds.size());
			int remainder = (int) (nb % poolIds.size());
			for (auto &&id: poolIds) {
				tmpIdent.clear();

				int nbToAlloc = blkPerPool;
				if (0 < remainder) {
					nbToAlloc++;
					remainder--;
				}

				switch (type) {
					case INODE:
						this->pools[id]->addInode(info, tmpIdent, nbToAlloc);
						break;
					case DIR_BLOCK:
						this->pools[id]->addDirBlock(info, tmpIdent, nbToAlloc);
						break;
					case BLOCK:
						this->pools[id]->addBlock(info, tmpIdent, nbToAlloc);
				}

				for (auto &&ident: tmpIdent) {
					ident.poolId = id;
					idents.push_back(ident);
				}
			}
		}

		return ret;
	}

	bool PoolManager::isLocked(const ident_t &id, const PoolManager::queryType &type) {
		mutex *mu = nullptr;
		set<ident_t> *lSet = nullptr;

		switch (type) {
			case INODE:
				mu = &this->inodeMutex;
				lSet = &this->lockedInodes;
				break;
			case DIR_BLOCK:
				mu = &this->dirMutex;
				lSet = &this->lockedDirBlock;
				break;
			case BLOCK:
				mu = &this->blockMutex;
				lSet = &this->lockedBlocks;
				break;
		}

		if (nullptr == mu || nullptr == lSet)
			return false;

		unique_lock<mutex> lk(*mu);
		return lSet->find(id) != lSet->end();
	}

	bool PoolManager::lock(const ident_t &id, const PoolManager::queryType &type) {
		mutex *mu = nullptr;
		set<ident_t> *lSet = nullptr;

		switch (type) {
			case INODE:
				mu = &this->inodeMutex;
				lSet = &this->lockedInodes;
				break;
			case DIR_BLOCK:
				mu = &this->dirMutex;
				lSet = &this->lockedDirBlock;
				break;
			case BLOCK:
				mu = &this->blockMutex;
				lSet = &this->lockedBlocks;
				break;
		}

		assert(nullptr != mu && nullptr != lSet);

		unique_lock<mutex> lk(*mu);
		if (lSet->find(id) != lSet->end())
			return false;

		lSet->insert(id);

		return true;
	}

	bool PoolManager::hasMoved(const ident_t &id, ident_t &newId, const PoolManager::queryType &type) {
		mutex *mu = nullptr;
		map<ident_t, ident_t> *transMap = nullptr;

		switch (type) {
			case INODE:
				mu = &this->inodeTransMutex;
				transMap = &this->inodeTranslateMap;
				break;
			case DIR_BLOCK:
				mu = &this->dirTransMutex;
				transMap = &this->dirBlockTranslateMap;
				break;
			case BLOCK:
				mu = &this->blockTransMutex;
				transMap = &this->blockTranslateMap;
				break;
		}

		assert(mu != nullptr);
		assert(transMap != nullptr);

		unique_lock<mutex> lk(*mu);
		if (transMap->find(id) != transMap->end()) {
			newId = (*transMap)[id];
			return true;
		}

		return false;
	}

	int PoolManager::getValidPools(const ruleInfo_t &info, std::vector<uint32_t> &poolIds) {
		for (auto &&item: this->rules) {
			if (item.second->satisfyRules(info))
				poolIds.push_back(item.first);
		}

		return SUCCESS;
	}


}  // namespace mtfs
