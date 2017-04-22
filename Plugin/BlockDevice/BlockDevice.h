#ifndef PLUGINSYSTEM_BLOCK_DEVICE_H
#define PLUGINSYSTEM_BLOCK_DEVICE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "Plugin.h"
#include <FileStorage/structs.h>

namespace PluginSystem {
	class BlockDevice : public Plugin {
	public:
		bool init(int blockSize, std::vector<std::string> params) override;

		bool addInode(std::uint64_t &inodeId) override;

		bool delInode(std::uint64_t inodeId) override;

		bool readInode(std::uint64_t inodeId, FileStorage::inode_st &inode) override;

		bool writeInode(std::uint64_t inodeId, FileStorage::inode_st &inode) override;

		bool addBlock(std::uint64_t &blockId) override;

		bool delBlock(std::uint64_t blockId) override;

		bool readBlock(std::uint64_t blockId, std::uint8_t *buffer) override;

		bool writeBlock(std::uint64_t blockId, std::uint8_t *buffe) override;

		bool readSuperblock(FileStorage::superblock_t &superblock) override;

		bool writeSuperblock(FileStorage::superblock_t superblock) override;
	};

}  // namespace Plugin
#endif
