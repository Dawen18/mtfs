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

	int PoolManager::add(const ruleInfo_t &info, std::vector<ident_t> &ids, const Acces::queryType type, const int nb) {
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

	int PoolManager::del(const ident_t &id, const Acces::queryType type) {
		return this->pools[id.poolId]->del(id.volumeId, id.id, type);
	}

	int PoolManager::get(const ident_t &id, void *data, const Acces::queryType type) {
		return this->pools[id.poolId]->get(id.volumeId, id.id, data, type);
	}

	int PoolManager::put(const ident_t &id, const void *data, const Acces::queryType type) {
		return this->pools[id.poolId]->put(id.volumeId, id.id, data, type);
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
			case DATA_BLOCK:
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
