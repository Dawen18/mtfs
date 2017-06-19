#ifndef FILESTORAGE_VOLUME_H
#define FILESTORAGE_VOLUME_H

#include <string>
#include <vector>

#include <pluginSystem/Plugin.h>
#include <mtfs/structs.h>
#include <rapidjson/document.h>
#include "Acces.h"

namespace mtfs {
	class Volume {
	public:
		static constexpr const char *VOLUMES = "volumes";

	private:

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

		bool getSuperblock(mtfs::superblock_t &superblock);

		bool putSuperblock(superblock_t &superblock);

		int add(uint64_t &id, const Acces::queryType type);

		int add(std::vector<uint64_t> &ids, const int nb, const Acces::queryType type);

		int del(const uint64_t &id, const Acces::queryType type);

		int get(const uint64_t &id, void *data, Acces::queryType type);

		int put(const uint64_t &id, const void *data, Acces::queryType type);


	private:

		bool uptadeLastAcces(uint64_t id, std::map<uint64_t, std::vector<uint64_t>> map);
	};

}  // namespace mtfs
#endif
