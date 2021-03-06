/**
 * \file PoolManager.h
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
		std::recursive_mutex inodeTransMutex;
		std::map<ident_t, ident_t> inodeTranslateMap;

		std::mutex dirMutex;
		std::set<ident_t> lockedDirBlock;
		std::recursive_mutex dirTransMutex;
		std::map<ident_t, ident_t> dirBlockTranslateMap;

		std::mutex blockMutex;
		std::set<ident_t> lockedBlocks;
		std::recursive_mutex blockTransMutex;
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

		void dumpTranslateMap(const int &nb, const blockType &type);

		const int moveBlk(const ident_t &old, const ident_t &cur, const blockType &type);
	};

}  // namespace mtfs
#endif
