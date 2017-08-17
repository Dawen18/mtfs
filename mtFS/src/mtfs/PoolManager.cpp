/**
 * \file PoolManager.cpp
 * \brief
 * \author David Wittwer
 * \version 0.0.1
 * \copyright GNU Publis License V3
 *
 * This file is part of MTFS.

    MTFS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <mtfs/PoolManager.h>
#include <utils/Logger.h>

namespace mtfs {
	using namespace std;

	PoolManager::~PoolManager() {
//		dumpTranslateMap(INT_MAX, INODE);
//		dumpTranslateMap(INT_MAX, DIR_BLOCK);
//		dumpTranslateMap(INT_MAX, DATA_BLOCK);
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

		if (!this->isLocked(newId, type)) {
			string message = "get ";

			switch (type) {
				case INODE:
					message += "inode ";
					break;
				case DIR_BLOCK:
					message += "dir block ";
					break;
				case DATA_BLOCK:
					message += "dat block ";
					break;
				case SUPERBLOCK:
					message += "superblock ";
					break;
			}

			message += newId.toString();

			Logger::getInstance()->log("POOL_MANAGER", message, Logger::L_DEBUG);
			ret = this->pools[newId.poolId]->get(newId.volumeId, newId.id, data, type);
		}

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

//		this->dumpTranslateMap(10, type);

		for (auto &&pool: this->pools) {
			map<ident_t, ident_t> tmpMovedBlk;
			pool.second->doMigration(tmpMovedBlk, unsatisfyBlk, type);
			for (auto &&item :tmpMovedBlk) {
				ident_t key = item.first;
				ident_t val = item.second;

				key.poolId = pool.first;
				val.poolId = pool.first;
				recursive_mutex *mu = nullptr;
				map<ident_t, ident_t> *map = nullptr;
				switch (type) {
					case INODE:
						mu = &this->inodeTransMutex;
						map = &this->inodeTranslateMap;
						break;
					case DIR_BLOCK:
						mu = &this->dirTransMutex;
						map = &this->dirBlockTranslateMap;
						break;
					case DATA_BLOCK:
						mu = &this->blockTransMutex;
						map = &this->blockTranslateMap;
						break;
					case SUPERBLOCK:
						continue;
						break;
				}
				unique_lock<recursive_mutex> lk(*mu);
				map->emplace(key, val);
			}

			Logger::getInstance()->log("Poolmanager", "endMig", Logger::level::L_DEBUG);
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
		recursive_mutex *mu = nullptr;
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
			default:
				return false;
		}

		assert(mu != nullptr);
		assert(transMap != nullptr);

		unique_lock<recursive_mutex> lk(*mu);
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

	void PoolManager::dumpTranslateMap(const int &nb, const blockType &type) {

		recursive_mutex *mu;
		map<ident_t, ident_t> *tm;
		switch (type) {
			case INODE:
				mu = &this->inodeTransMutex;
				tm = &this->inodeTranslateMap;
				break;
			case DIR_BLOCK:
				mu = &this->dirTransMutex;
				tm = &this->dirBlockTranslateMap;
				break;
			case DATA_BLOCK:
				mu = &this->blockTransMutex;
				tm = &this->blockTranslateMap;
				break;
			default:
				return;
		}

		unique_lock<recursive_mutex> lk(*mu);
		int i = 0;
		for (auto &&item :*tm) {
			if (nb == i)
				break;
			if (SUCCESS == this->moveBlk(item.first, item.second, type)) {
				tm->erase(item.first);
			}
			i++;
		}
	}

	const int PoolManager::moveBlk(const ident_t &old, const ident_t &cur, const blockType &type) {
		blockInfo_t metas{};
		this->pools[cur.poolId]->getMetas(cur.volumeId, cur.id, metas, type);
		dirBlock_t dirBlock{};

		switch (type) {
			case INODE:
				this->get(metas.referenceId.front(), &dirBlock, DIR_BLOCK);
				for (auto &&item :dirBlock.entries) {
					for (auto &i : item.second) {
						if (old == i)
							i = cur;
					}
				}
				for (auto &&ref :metas.referenceId) {
					this->put(ref, &dirBlock, DIR_BLOCK);
				}
				break;
			case DIR_BLOCK:
			case DATA_BLOCK:
				inode_t inode{};
				this->get(metas.referenceId.front(), &inode, INODE);

				for (auto &&blks :inode.dataBlocks) {
					for (auto &blk :blks) {
						if (old == blk)
							blk = cur;
					}
				}
				for (auto &&ref :metas.referenceId) {
					this->put(ref, &inode, INODE);
				}
				break;
		}

		return SUCCESS;
	}

}  // namespace mtfs
