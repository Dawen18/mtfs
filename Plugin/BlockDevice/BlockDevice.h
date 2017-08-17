/**
 * \file BlockDevice.h
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

#ifndef PLUGINSYSTEM_BLOCK_DEVICE_H
#define PLUGINSYSTEM_BLOCK_DEVICE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <cassert>

#include <pluginSystem/Plugin.h>
#include <mtfs/structs.h>
#include <map>
#include <queue>
#include <mutex>

namespace pluginSystem {
	class BlockDevice : public Plugin {
		static constexpr const char *INODES_DIR = "inodes";
		static constexpr const char *BLOCKS_DIR = "blocks";
		static constexpr const char *DIR_BLOCKS_DIR = "dirBlocks";
		static constexpr const char *METAS_DIR = "metas";
		static constexpr const char *INODE_METAS_DIR = "metas/inodes";
		static constexpr const char *DIR_BLOCK_METAS_DIR = "metas/dirBlocks";
		static constexpr const char *BLOCK_METAS_DIR = "metas/blocks";

		static const int SUCCESS = 0;

	public:
		static constexpr const char *NAME = "block";

		BlockDevice();

		virtual ~BlockDevice();

		bool attach(std::map<std::string, std::string> params) override;

		bool detach() override;

		int add(uint64_t *id, const mtfs::blockType &type) override;

		int del(const uint64_t &id, const mtfs::blockType &type) override;

		int get(const uint64_t &id, void *data, const mtfs::blockType &type, bool metas) override;

		int put(const uint64_t &id, const void *data, const mtfs::blockType &type, bool metas) override;

		bool getSuperblock(mtfs::superblock_t &superblock) override;

		bool putSuperblock(const mtfs::superblock_t &superblock) override;

		std::string getName() override;

	private:
		int blockSize;
		std::string mountpoint;
		std::string devicePath;
		std::string fsType;

		std::mutex inodeMutex;
		std::vector<uint64_t> freeInodes;
		uint64_t nextFreeInode;

		std::mutex dirBlockMutex;
		std::vector<uint64_t> freeDirBlocks;
		uint64_t nextFreeDirBlock;

		std::mutex blockMutex;
		std::vector<uint64_t> freeBlocks;
		uint64_t nextFreeBlock;


		int addInode(uint64_t *inodeId);

		int addDirBlock(uint64_t *id);

		int addBlock(uint64_t *blockId);

		int delInode(const uint64_t &inodeId);

		int getInode(const uint64_t &inodeId, mtfs::inode_st &inode);

		int putInode(const uint64_t &inodeId, const mtfs::inode_st &inode);

		int getInodeMetas(const uint64_t &inodeId, mtfs::blockInfo_t &metas);

		int putInodeMetas(const uint64_t &inodeId, const mtfs::blockInfo_t &metas);

		int delDirBlock(const uint64_t &id);

		int getDirBlock(const uint64_t &id, mtfs::dirBlock_t &block);

		int putDirBlock(const uint64_t &id, const mtfs::dirBlock_t &block);

		int getDirBlockMetas(const uint64_t &id, mtfs::blockInfo_t &metas);

		int putDirBlockMetas(const uint64_t &id, const mtfs::blockInfo_t &metas);

		int delBlock(const uint64_t &blockId);

		int getBlock(const uint64_t &blockId, std::uint8_t *buffer);

		int putBlock(const uint64_t &blockId, const uint8_t *buffer);

		int getBlockMetas(const uint64_t &blockId, mtfs::blockInfo_t &metas);

		int putBlockMetas(const uint64_t &blockId, const mtfs::blockInfo_t &metas);


		void initDirHierarchie();

		void initInodes();

		void initDirBlocks();

		void initBlocks();

		void writeMetas();

		void logError(std::string message);

		bool dirExists(std::string path);

		int createFile(std::string path);

		int deleteFile(std::string path);

		int getMetas(const std::string &filename, mtfs::blockInfo_t &infos);

		int putMetas(const std::string &filename, const mtfs::blockInfo_t &infos);

	};

}  // namespace Plugin
#endif
