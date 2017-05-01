#ifndef PLUGINSYSTEM_BLOCK_DEVICE_H
#define PLUGINSYSTEM_BLOCK_DEVICE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include <PluginSystem/Plugin.h>
#include <FileStorage/structs.h>
#include <map>
#include <queue>

namespace PluginSystem {
	class BlockDevice : public Plugin {
		static constexpr const char *INODES_DIR = "inodes";
		static constexpr const char *BLOCKS_DIR = "blocks";
		static constexpr const char *METAS_DIR = "metas";

	public:
		BlockDevice();

		std::vector<std::string> getInfos() override;

		bool attach(std::map<std::string, std::string> params) override;

		bool detach() override;

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

	private:
		std::string mountpoint;
		std::string devicePath;
		std::string fsType;
		int blockSize;
		std::vector<uint64_t> freeInodes;
		uint64_t nextFreeInode;

		void initDirHierarchie();

		void initInodes();

		void writeMetas();

		void logError(std::string message);

		bool dirExists(std::string path);

		bool createFile(std::string path);

		bool deleteFile(std::string path);

	};

}  // namespace Plugin
#endif
