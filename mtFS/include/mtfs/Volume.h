#ifndef FILESTORAGE_VOLUME_H
#define FILESTORAGE_VOLUME_H

#include <string>
#include <vector>

#include <pluginSystem/Plugin.h>
#include <mtfs/structs.h>
#include <rapidjson/document.h>

namespace mtfs {
	class Volume : public PluginSystem::Plugin {
	public:
		static constexpr const char *VOLUMES = "volumes";

	private:
		PluginSystem::Plugin *plugin;
		std::map<uint64_t, uint64_t> blocksAccess;
		std::map<uint64_t, uint64_t> inodesAccess;


	public:
		static bool validate(const rapidjson::Value &volume);

		Volume(PluginSystem::Plugin *plugin);

		bool setTimeLimits(int low, int high);

		void getBlockInfo(uint64_t blockId, blockInfo_st &info);

		void setBlockInfo(uint64_t blockId, blockInfo_st &info);

		std::vector<ident_st> getBlocksBelowLimit();

		std::vector<ident_st> getBlocksAboveLimit();

		std::vector<ident_st> getInodesBelowLimit();

		std::vector<ident_st> getInodesAboveLimit();

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

		bool writeSuperblock(superblock_t &superblock) override;

	private:
		bool uptadeLastAcces(uint64_t id, std::map<uint64_t, std::vector<uint64_t>> map);
	};

}  // namespace mtfs
#endif
