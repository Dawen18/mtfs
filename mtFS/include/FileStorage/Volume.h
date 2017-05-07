#ifndef FILESTORAGE_VOLUME_H
#define FILESTORAGE_VOLUME_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include <PluginSystem/Plugin.h>
#include <FileStorage/structs.h>

namespace FileStorage {
	class Volume : public PluginSystem::Plugin {
	private:
		PluginSystem::Plugin *plugin;

		std::map<uint64_t, std::vector<uint64_t>> blocksAccess;

		std::map<uint64_t, std::vector<uint64_t>> inodesAccess;


	private:
		bool uptadeLastAcces(uint64_t id, std::map<uint64_t, std::vector<uint64_t>> map);

	public:
		bool setTimeLimits(int low, int high);

		void getBlockInfo(uint64_t blockId, blockInfo_st &info);

		void setBlockInfo(uint64_t blockId, blockInfo_st &info);

		std::vector<ident_st> getBlocksBelowLimit();

		std::vector<ident_st> getBlocksAboveLimit();

		std::vector<ident_st> getInodesBelowLimit();

		std::vector<ident_st> getInodesAboveLimit();

	};

}  // namespace FileStorage
#endif
