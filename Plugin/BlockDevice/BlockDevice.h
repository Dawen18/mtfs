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
#include <mutex>

namespace pluginSystem {
	class BlockDevice : public Plugin {
		static constexpr const char *INODES_DIR = "inodes";
		static constexpr const char *BLOCKS_DIR = "blocks";
		static constexpr const char *DIR_BLOCKS_DIR = "dirBlocks";
		static constexpr const char *METAS_DIR = "metas";
		static constexpr const char *BLOCK_METAS_DIR = "metas/block";

		static const int SUCCESS = 0;

	public:
		BlockDevice();

		virtual ~BlockDevice();

		bool attach(std::map<std::string, std::string> params) override;

		bool detach() override;

		int addInode(uint64_t *inodeId) override;

		int delInode(const uint64_t &inodeId) override;

		int getInode(const uint64_t &inodeId, mtfs::inode_st &inode) override;

		int putInode(const uint64_t &inodeId, const mtfs::inode_st &inode) override;

		int addDirBlock(uint64_t *id) override;

		int delDirBlock(const uint64_t &id) override;

		int getDirBlock(const uint64_t &id, mtfs::dirBlock_t &block) override;

		int putDirBlock(const uint64_t &id, const mtfs::dirBlock_t &block) override;

		int addBlock(uint64_t *blockId) override;

		int delBlock(const uint64_t &blockId) override;

		int getBlock(const uint64_t &blockId, std::uint8_t *buffer) override;

		int putBlock(const uint64_t &blockId, const uint8_t *buffer) override;

		bool getBlockMetas(const uint64_t &blockId, mtfs::blockInfo_t &metas) override;

		bool putBlockMetas(const uint64_t &blockId, const mtfs::blockInfo_t &metas) override;

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


		void initDirHierarchie();

		void initInodes();

		void initDirBlocks();

		void initBlocks();

		void writeMetas();

		void logError(std::string message);

		bool dirExists(std::string path);

		int createFile(std::string path);

		int deleteFile(std::string path);

	};

}  // namespace Plugin
#endif
