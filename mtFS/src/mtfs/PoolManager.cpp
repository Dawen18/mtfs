#include <mtfs/PoolManager.h>
#include <utils/Logger.h>

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

	int PoolManager::add(const ruleInfo_t &info, std::vector<ident_t> &ids, const blockType type,
						 const size_t nb) {
		int ret;
		vector<uint32_t> poolIds;
		if (0 != (ret = this->getValidPools(info, poolIds))) {
			return ret;
		}
		vector<ident_t> tmpIdent;

		if (nb <= poolIds.size()) {
			for (auto &&id: poolIds) {
				tmpIdent.clear();

				this->pools[id]->add(info, tmpIdent, type);

				for (auto &&ident: tmpIdent) {
					ident.poolId = id;
					ids.push_back(ident);
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

				this->pools[id]->add(info, tmpIdent, type, nbToAlloc);

				for (auto &&ident: tmpIdent) {
					ident.poolId = id;
					ids.push_back(ident);
				}
			}
		}

		return ret;
	}

	int PoolManager::del(const ident_t &id, const blockType type) {
		int ret = IS_LOCKED;

		ident_t newId = id;
		this->hasMoved(id, newId, type);

		if (!this->isLocked(newId, type))
			ret = this->pools[newId.poolId]->del(newId.volumeId, newId.id, type);

		return ret;
	}

	int PoolManager::get(const ident_t &id, void *data, const blockType type) {
		int ret = IS_LOCKED;

		ident_t newId = id;
		this->hasMoved(id, newId, type);

		if (!this->isLocked(newId, type))
			ret = this->pools[newId.poolId]->get(newId.volumeId, newId.id, data, type);

		return ret;
	}

	int PoolManager::put(const ident_t &id, const void *data, const blockType type) {
		int ret = IS_LOCKED;

		ident_t newId = id;
		this->hasMoved(id, newId, type);

		if (this->pools.end() == this->pools.find(newId.poolId))
			throw out_of_range("Invalid pool id " + newId.poolId);

		if (this->lock(newId, type)) {
			ret = this->pools[newId.poolId]->put(newId.volumeId, newId.id, data, type);
			this->unlock(newId, type);
		}

		return ret;
	}

	int PoolManager::getMetas(const ident_t &id, blockInfo_t &metas, const blockType type) {
		int ret = IS_LOCKED;

		ident_t newId = id;
		this->hasMoved(id, newId, type);

		if (!this->isLocked(newId, type))
			ret = this->pools[newId.poolId]->getMetas(newId.volumeId, newId.id, metas, type);

		return ret;
	}

	int PoolManager::putMetas(const ident_t &id, const blockInfo_t &metas, const blockType type) {
		int ret = IS_LOCKED;

		ident_t newId = id;
		this->hasMoved(id, newId, type);

		if (this->pools.end() == this->pools.find(newId.poolId))
			throw out_of_range("Invalid pool id " + newId.poolId);

		if (this->lock(newId, type)) {
			ret = this->pools[newId.poolId]->putMetas(newId.volumeId, newId.id, metas, type);
			this->unlock(newId, type);
		}

		return ret;
	}

	void PoolManager::doMigration(const blockType type) {
		vector<ident_t> unsatisfyBlk;

		for (auto &&pool: this->pools) {
			map<ident_t, ident_t> tmpMovedBlk;
			pool.second->doMigration(tmpMovedBlk, unsatisfyBlk, type);
			for (auto &&item :tmpMovedBlk) {
				ident_t key = item.first;
				ident_t val = item.second;

				key.poolId = pool.first;
				val.poolId = pool.first;
				this->blockTranslateMap.insert(make_pair(key, val));
			}

			Logger::getInstance()->log("Poolmanager", "endMig",Logger::level::L_DEBUG);
		}
	}

	bool PoolManager::isLocked(const ident_t &id, const blockType &type) {
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
			case DATA_BLOCK:
				mu = &this->blockMutex;
				lSet = &this->lockedBlocks;
				break;
		}

		if (nullptr == mu || nullptr == lSet)
			return false;

		unique_lock<mutex> lk(*mu);
		return lSet->find(id) != lSet->end();
	}

	bool PoolManager::lock(const ident_t &id, const blockType &type) {
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
			case DATA_BLOCK:
				mu = &this->blockMutex;
				lSet = &this->lockedBlocks;
				break;
			default:
//				TODO log error
				return false;
				break;
		}

		unique_lock<mutex> lk(*mu);
		if (lSet->find(id) != lSet->end())
			return false;

		lSet->insert(id);

		return true;
	}

	bool PoolManager::unlock(const ident_t &id, const blockType &type) {
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
			case DATA_BLOCK:
				mu = &this->blockMutex;
				lSet = &this->lockedBlocks;
				break;
			default:
//				TODO log error
				return false;
				break;
		}

		unique_lock<mutex> lk(*mu);
		if (lSet->find(id) == lSet->end())
			return false;

		lSet->erase(id);

		return true;
	}

	bool PoolManager::hasMoved(const ident_t &id, ident_t &newId, const blockType &type) {
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
			case DATA_BLOCK:
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
