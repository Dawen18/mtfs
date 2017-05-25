#ifndef PLUGINSYSTEM_BLOCK_DEVICE_H
#define PLUGINSYSTEM_BLOCK_DEVICE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include <pluginSystem/Plugin.h>
#include <mtfs/structs.h>
#include <map>
#include <queue>

namespace pluginSystem {
	class BlockDevice : public Plugin {
		static constexpr const char *INODES_DIR = "inodes";
		static constexpr const char *BLOCKS_DIR = "blocks";
		static constexpr const char *METAS_DIR = "metas";

	public:
		BlockDevice();

		virtual ~BlockDevice();

		std::vector<std::string> getInfos() override;

		bool attach(std::map<std::string, std::string> params) override;

		bool detach() override;

		bool addInode(std::uint64_t &inodeId) override;

		bool delInode(std::uint64_t inodeId) override;

		bool readInode(std::uint64_t inodeId, mtfs::inode_st &inode) override;

		bool writeInode(std::uint64_t inodeId, mtfs::inode_st &inode) override;

		bool addBlock(std::uint64_t &blockId) override;

		bool delBlock(std::uint64_t blockId) override;

		bool readBlock(std::uint64_t blockId, std::uint8_t *buffer) override;

		bool writeBlock(std::uint64_t blockId, std::uint8_t *buffer) override;

		bool readSuperblock(mtfs::superblock_t &superblock) override;

		bool writeSuperblock(mtfs::superblock_t &superblock) override;

	private:
		int blockSize;
		std::string mountpoint;
		std::string devicePath;
		std::string fsType;
		
		std::vector<uint64_t> freeInodes;
		uint64_t nextFreeInode;
		
		std::vector<uint64_t > freeBlocks;
		uint64_t nextFreeBlock;
		

		void initDirHierarchie();

		void initInodes();
		
		void initBlocks();

		void writeMetas();

		void logError(std::string message);

		bool dirExists(std::string path);

		bool createFile(std::string path);

		bool deleteFile(std::string path);

	};

}  // namespace Plugin
#endif
