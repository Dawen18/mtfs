#ifndef FILESTORAGE_VOLUME_H
#define FILESTORAGE_VOLUME_H

#include <string>
#include <vector>

#include <rapidjson/document.h>
#include <mtfs/structs.h>
#include <pluginSystem/Plugin.h>
#include "Visitor.h"

namespace mtfs {
	class Volume {
		friend class Migrator;

	public:
		static constexpr const char *VOLUMES = "volumes";

	private:

		pluginSystem::Plugin *plugin;

		std::map<uint64_t, uint64_t> inodesAccess;
		std::map<uint64_t, uint64_t> dirBlockAccess;
		std::map<uint64_t, uint64_t> blocksAccess;


	public:
		virtual ~Volume();

		static bool validate(const rapidjson::Value &volume);

		static void
		structToJson(const volume_t &volume, rapidjson::Value &dest, rapidjson::Document::AllocatorType &allocator);

		static void jsonToStruct(rapidjson::Value &src, volume_t &volume);

		Volume(pluginSystem::Plugin *plugin);

		void getBlockInfo(uint64_t blockId, blockInfo_st &info);

		void setBlockInfo(uint64_t blockId, blockInfo_st &info);

		int add(uint64_t &id, const queryType type);

		int add(std::vector<uint64_t> &ids, const int nb, const queryType type);

		int del(const uint64_t &id, const queryType type);

		int get(const uint64_t &id, void *data, queryType type);

		int put(const uint64_t &id, const void *data, queryType type);

		void accept(class Visitor *v);
	private:

		bool updateLastAccess(const uint64_t &id, const queryType type);
	};

}  // namespace mtfs
#endif
