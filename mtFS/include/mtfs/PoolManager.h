#ifndef FILESTORAGE_POOL_MANAGER_H
#define FILESTORAGE_POOL_MANAGER_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "mtfs/DirectoryBlockAccess.h"
#include "mtfs/BlockAccess.h"
#include "mtfs/InodeAcces.h"
#include "mtfs/InodeCache.h"
#include "mtfs/BlockCache.h"
#include "mtfs/DirectoryEntryCache.h"
#include "mtfs/Pool.h"
#include "Mtfs.h"

namespace mtfs {
	class PoolManager : public DirectoryBlockAccess, public BlockAccess, public InodeAcces {
	public:
		static const int SUCCESS = 0;
		static const int POOL_ID_EXIST = 1;
		static const int NO_VALID_POOL = 2;
		static const int IS_LOCKED = 3;

	private:
		enum queryType {
			INODE,
			DIR_BLOCK,
			BLOCK,
		};

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

		int addBlock(const ruleInfo_t &infos, std::vector<ident_t> &ident, const int nb) override;

		bool delBlock(inode_st &inode) override;

		bool getBlock(inode_st &inode, ident_t &blockId, uint8_t *buffer) override;

		int putBlock(const ident_t &blockId, uint8_t *buffer) override;

		int addDirBlock(const ruleInfo_t &infos, std::vector<ident_t> &blockId, const int nb) override;

		int delDirBlock(const ident_t &blockId) override;

		int getDirBlock(const ident_t &blockId, dirBlock_t &block) override;

		int putDirBlock(const ident_t &blockId, const dirBlock_t &block) override;

		int addInode(const ruleInfo_t &info, std::vector<ident_t> &idents, const int nb) override;

		int delInode(const ident_t &inodeId) override;

		int getInode(const ident_t &inodeId, inode_st &inode) override;

		int putInode(const ident_t &inodeId, const inode_t &inode) override;

	private:

		int add(const ruleInfo_t &info, std::vector<ident_t> &idents, const int nb, const queryType type);

		bool isLocked(const ident_t &id, const queryType &type);

		bool lock(const ident_t &id, const queryType &type);

		bool hasMoved(const ident_t &id, ident_t &newId, const queryType &type);

		int getValidPools(const ruleInfo_t &info, std::vector<uint32_t> &poolIds);
	};

}  // namespace mtfs
#endif
