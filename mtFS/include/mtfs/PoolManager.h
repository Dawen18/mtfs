#ifndef FILESTORAGE_POOL_MANAGER_H
#define FILESTORAGE_POOL_MANAGER_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "mtfs/Acces.h"
#include "mtfs/InodeCache.h"
#include "mtfs/BlockCache.h"
#include "mtfs/DirectoryEntryCache.h"
#include "mtfs/Pool.h"
#include "Mtfs.h"

namespace mtfs {
	class PoolManager : public Acces {
	public:
		static const int SUCCESS = 0;
		static const int POOL_ID_EXIST = 1;
		static const int NO_VALID_POOL = 2;
		static const int IS_LOCKED = 3;

	private:


		std::map<uint32_t, Pool *> pools;
		std::map<uint32_t, Rule *> rules;

		std::mutex inodeMutex;
		std::set<ident_t> lockedInodes;
		std::mutex inodeTransMutex;
		std::map<ident_t, ident_t> inodeTranslateMap;

		std::mutex dirMutex;
		std::set<ident_t> lockedDirBlock;
		std::mutex dirTransMutex;
		std::map<ident_t, ident_t> dirBlockTranslateMap;

		std::mutex blockMutex;
		std::set<ident_t> lockedBlocks;
		std::mutex blockTransMutex;
		std::map<ident_t, ident_t> blockTranslateMap;


	public:
		virtual ~PoolManager();

		int addPool(uint32_t poolId, Pool *pool, Rule *rule);

		int add(const ruleInfo_t &info, std::vector<ident_t> &ids, const Acces::queryType type, const size_t nb) override;

		int del(const ident_t &id, const Acces::queryType type) override;

		int get(const ident_t &id, void *data, const Acces::queryType type) override;

		int put(const ident_t &id, const void *data, const Acces::queryType type) override;

	private:

		bool isLocked(const ident_t &id, const queryType &type);

		bool lock(const ident_t &id, const queryType &type);

		bool hasMoved(const ident_t &id, ident_t &newId, const queryType &type);

		int getValidPools(const ruleInfo_t &info, std::vector<uint32_t> &poolIds);
	};

}  // namespace mtfs
#endif
