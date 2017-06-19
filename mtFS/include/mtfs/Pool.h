#ifndef FILESTORAGE_POOL_H
#define FILESTORAGE_POOL_H

#include <string>
#include <vector>
#include <mtfs/structs.h>
#include <mtfs/Volume.h>
#include <mtfs/Rule.h>
#include <pluginSystem/Plugin.h>
#include "Acces.h"

namespace mtfs {
	class Pool {
	public:
		static constexpr const char *POOLS = "pools";

//		STATUS CODE
		static const int SUCCESS = 0;
		static const int VOLUME_ID_EXIST = 1;
		static const int NO_VALID_VOLUME = 2;


	private:

		std::map<uint32_t, Volume *> volumes;

		std::map<uint32_t, Rule *> rules;


	public:
		virtual ~Pool();

		static bool validate(const rapidjson::Value &pool);

		static void
		structToJson(const pool_t &pool, rapidjson::Value &dest, rapidjson::Document::AllocatorType &allocator);

		static void jsonToStruct(rapidjson::Value &src, pool_t &pool);

		int addVolume(uint32_t volumeId, Volume *volume, Rule *rule);

		int add(const ruleInfo_t &info, std::vector<ident_t> &idents, const Acces::queryType type, const int nb = 1);

		int del(const uint32_t &volumeId, const uint64_t &id, const Acces::queryType type);

		int get(const uint32_t &volumeId, const uint64_t &id, void *data, const Acces::queryType type);

		int put(const uint32_t &volumeId, const uint64_t &id, const void *data, const Acces::queryType type);


	private:

		int getValidVolumes(const ruleInfo_t &info, std::vector<uint32_t> &volumeIds);
	};

}  // namespace mtfs
#endif
