#include <string>
#include <vector>
#include <list>
#include <iostream>

#include "BlockDevice.h"

namespace PluginSystem {
	bool BlockDevice::init(int blockSize, std::vector<std::string> params) {
		return false;
	}

	bool BlockDevice::addInode(std::uint64_t &inodeId) {
		return false;
	}

	bool BlockDevice::delInode(std::uint64_t inodeId) {
		return false;
	}

	bool BlockDevice::readInode(std::uint64_t inodeId, FileStorage::inode_st &inode) {
		return false;
	}

	bool BlockDevice::writeInode(std::uint64_t inodeId, FileStorage::inode_st &inode) {
		return false;
	}

	bool BlockDevice::addBlock(std::uint64_t &blockId) {
		return false;
	}

	bool BlockDevice::delBlock(std::uint64_t blockId) {
		return false;
	}

	bool BlockDevice::readBlock(std::uint64_t blockId, std::uint8_t *buffer) {
		return false;
	}

	bool BlockDevice::writeBlock(std::uint64_t blockId, std::uint8_t *buffe) {
		return false;
	}

	bool BlockDevice::readSuperblock(FileStorage::superblock_t &superblock) {
		return false;
	}

	bool BlockDevice::writeSuperblock(FileStorage::superblock_t superblock) {
		return false;
	}

}  // namespace Plugin
