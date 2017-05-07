#ifndef FILESTORAGE_POOL_H
#define FILESTORAGE_POOL_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>
#include <FileStorage/structs.h>
#include "FileStorage/Volume.h"
#include "FileStorage/Rule.h"
#include "PluginSystem/Plugin.h"

namespace FileStorage {
	class Pool {
	private:
		std::map<uint32_t, Volume> volumes;

		std::map<uint32_t, Rule> rules;

		int blockSize;

		Rule *rule;


	private:
		bool chooseVolume(ruleInfo_t &info, PluginSystem::Plugin *volume);

	public:
		bool addBlock(ruleInfo_t &info, uint16_t &volumeId, uint64_t &blockId);

		bool delBlock(uint16_t volumeId, uint64_t blockId);

		bool readBlock(uint16_t volumeId, uint64_t blockId, uint8_t *block);

		bool writeBlock(uint16_t volumeId, uint64_t blockId, uint8_t *block);

		bool addInode(ruleInfo_t &info, uint16_t &volumeId, uint64_t &inodeId);

		bool delInode(uint16_t volumeId, uint64_t inodeId);

		bool readInode(uint16_t volumeId, uint64_t inodeId);

		bool writeInode(uint16_t volumeId, uint64_t inodeId);

		std::vector<superblock_t> readSuperblocks();

		bool writeSuperblocks(superblock_t baseSuperblock);

		void setBlockInfo(ident_t blockId, blockInfo_t &info);

		void getBlockInfo(ident_t blockId, blockInfo_t &info);

		void moveBlocks(std::vector<move_t> &asMoved, std::vector<ident_t> &needOtherPool);

	};

}  // namespace FileStorage
#endif