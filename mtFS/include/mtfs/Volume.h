#ifndef FILESTORAGE_VOLUME_H
#define FILESTORAGE_VOLUME_H

#include <string>
#include <vector>

#include <pluginSystem/Plugin.h>
#include <mtfs/structs.h>
#include <rapidjson/document.h>

namespace mtfs {
	class Volume {
	public:
		static constexpr const char *VOLUMES = "volumes";

	private:
		enum queryType {
			INODE,
			DIR_BLOCK,
			BLOCK,
		};

		pluginSystem::Plugin *plugin;
		std::map<uint64_t, uint64_t> blocksAccess;
		std::map<uint64_t, uint64_t> inodesAccess;


	public:
		virtual ~Volume();

		static bool validate(const rapidjson::Value &volume);

		static void
		structToJson(const volume_t &volume, rapidjson::Value &dest, rapidjson::Document::AllocatorType &allocator);

		static void jsonToStruct(rapidjson::Value &src, volume_t &volume);

		Volume(pluginSystem::Plugin *plugin);

		bool setTimeLimits(int low, int high);

		void getBlockInfo(uint64_t blockId, blockInfo_st &info);

		void setBlockInfo(uint64_t blockId, blockInfo_st &info);

		std::vector<ident_st> getBlocksBelowLimit();

		std::vector<ident_st> getBlocksAboveLimit();

		std::vector<ident_st> getInodesBelowLimit();

		std::vector<ident_st> getInodesAboveLimit();

		int addInode(uint64_t &inodeId);

		int addInode(std::vector<uint64_t> &ids, const int nb);

		int delInode(const uint64_t &inodeId);

		int getInode(const uint64_t &inodeId, mtfs::inode_st &inode);

		int putInode(const uint64_t &inodeId, const inode_t &inode);

		int addDirBlock(uint64_t &blockId);

		int addDirBlock(std::vector<uint64_t> &ids, const int nb);

		int delDirBlock(const std::uint64_t &blockId);

		int getDirBlock(const std::uint64_t &blockId, dirBlock_t &block);

		int putDirBlock(const std::uint64_t &blockId, const dirBlock_t &block);

		int addBlock(std::uint64_t &blockId);

		int addBlock(std::vector<uint64_t> &ids, const int nb);

		bool delBlock(std::uint64_t blockId);

		bool getBlock(std::uint64_t blockId, std::uint8_t *buffer);

		int putBlock(const uint64_t blockId, const uint8_t *buffer);

		bool getSuperblock(mtfs::superblock_t &superblock);

		bool putSuperblock(superblock_t &superblock);

	private:
		int add(std::vector<uint64_t> &ids, const int nb, const queryType &type);

		bool uptadeLastAcces(uint64_t id, std::map<uint64_t, std::vector<uint64_t>> map);
	};

}  // namespace mtfs
#endif
