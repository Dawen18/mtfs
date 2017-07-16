#ifndef FILESTORAGE_VOLUME_H
#define FILESTORAGE_VOLUME_H

#include <string>
#include <vector>
#include <climits>

#include <rapidjson/document.h>
#include <mtfs/structs.h>
#include <pluginSystem/Plugin.h>

namespace mtfs {
	class Volume {

	public:
		static constexpr const char *VOLUMES = "volumes";

	private:

		pluginSystem::Plugin *plugin;

		bool isTimeVolume;
		uint64_t minDelay;
		uint64_t maxDelay;

		std::map<uint64_t, uint64_t> inodesAccess;
		std::map<uint64_t, uint64_t> dirBlockAccess;
		std::map<uint64_t, uint64_t> blocksAccess;


	public:
		virtual ~Volume();

		void setMinDelay(uint64_t minDelay);

		void setMaxDelay(uint64_t maxDelay);

		void setIsTimeVolume(bool b);

		static bool validate(const rapidjson::Value &volume);

		static void
		structToJson(const volume_t &volume, rapidjson::Value &dest, rapidjson::Document::AllocatorType &allocator);

		static void jsonToStruct(rapidjson::Value &src, volume_t &volume);

		Volume(pluginSystem::Plugin *plugin);

		int add(uint64_t &id, const queryType type);

		int add(std::vector<uint64_t> &ids, const int nb, const queryType type);

		int del(const uint64_t &id, const queryType type);

		int get(const uint64_t &id, void *data, queryType type);

		int put(const uint64_t &id, const void *data, queryType type);

		int getMetas(const uint64_t &id, blockInfo_t &metas, queryType type);

		int putMetas(const uint64_t &id, const blockInfo_t &metas, queryType type);

		int getUnsatisfy(std::vector<blockInfo_t> &unsatisfy, const queryType &type, const int limit = INT_MAX);

	private:
		int getOutOfTime(std::vector<uint64_t> &blocks, const queryType &type);

		bool updateLastAccess(const uint64_t &id, const queryType type);
	};

}  // namespace mtfs
#endif
