#ifndef FILESTORAGE_POOL_MANAGER_H
#define FILESTORAGE_POOL_MANAGER_H

#include <string>
#include <vector>
#include <list>
#include <set>
#include <iostream>
#include <mutex>
#include <assert.h>

#include "mtfs/Acces.h"
#include "mtfs/Pool.h"

namespace mtfs {
	class Pool;

	class Acces;

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

		int add(const ruleInfo_t &info, std::vector<ident_t> &ids, const blockType type, const size_t nb) override;

		int del(const ident_t &id, const blockType type) override;

		int get(const ident_t &id, void *data, const blockType type) override;

		int put(const ident_t &id, const void *data, const blockType type) override;

		int getMetas(const ident_t &id, blockInfo_t &metas, const blockType type) override;

		int putMetas(const ident_t &id, const blockInfo_t &metas, const blockType type) override;

		void doMigration(const blockType type);

	private:

		bool isLocked(const ident_t &id, const blockType &type);

		bool lock(const ident_t &id, const blockType &type);

		bool unlock(const ident_t &id, const blockType &type);

		bool hasMoved(const ident_t &id, ident_t &newId, const blockType &type);

		int getValidPools(const ruleInfo_t &info, std::vector<uint32_t> &poolIds);

	};

}  // namespace mtfs
#endif
