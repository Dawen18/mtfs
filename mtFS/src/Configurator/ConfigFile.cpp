
#include <iostream>
#include "Configurator/ConfigFile.h"
#include "../../../Plugin/BlockDevice/BlockDevice.h"
#include <FileStorage/Mtfs.h>

namespace Configurator {
	using namespace std;
	using namespace rapidjson;
	using namespace FileStorage;

	ConfigFile::ConfigFile(const Document &d) : d(d) {
		loadPlugins();
	}

	void ConfigFile::loadPlugins() {
		plugins.push_back(new PluginSystem::BlockDevice());
	}

	PluginSystem::Plugin *ConfigFile::getPlugin(std::string name) {
		for (auto m:plugins) {
			vector<string> infos = m->getInfos();
			if (infos[0] == name)
				return m;
		}

		return NULL;
	}

	bool ConfigFile::validateFile() {
		if (!d.IsObject())
			return false;
		if (!d.HasMember(INODE_CACHE))
			return false;

		if (!d.HasMember(DIR_CACHE))
			return false;

		if (!d.HasMember(BLOCK_CACHE))
			return false;

		if (!d.HasMember(BLOCK_SIZE))
			return false;

		if (!d.HasMember(REDUNDANCY))
			return false;

		if (!d.HasMember(POOLS))
			return false;
		if (!d[POOLS].IsObject())
			return false;

		int migration = -1;
		if (d[POOLS].MemberCount() <= 0)
			return false;
		else if (d[POOLS].MemberCount() != 1) {
			if (!d.HasMember(MIGRATION))
				return false;

			migration = d[MIGRATION].GetInt();
		}

		for (auto &m: d[POOLS].GetObject()) {
			bool validRules;
			switch (migration) {
				case Mtfs::TIME_MIGRATION:
					validRules = validateTime(m.value);
					break;
				case Mtfs::RIGHT_MIGRATION:
					validRules = validateRight(m.value);
					break;
				default:
					validRules = (d[POOLS].MemberCount() == 1);
					break;
			}

			if (!validRules)
				return false;

			if (!validatePool(m.value))
				return false;
		}

		return true;
	}

	bool ConfigFile::validateRight(const Value &value) {
		return (value.HasMember(ALLOW_USER) || value.HasMember(DENY_USER) ||
				value.HasMember(ALLOW_GROUP) || value.HasMember(DENY_GROUP));
	}

	bool ConfigFile::validateTime(const Value &value) {
		return (value.HasMember(TIME_LOW_LIMIT) || value.HasMember(TIME_HIGH_LIMIT));
	}

	bool ConfigFile::validatePool(const rapidjson::Value &pool) {
		if (!pool.HasMember(VOLUMES))
			return false;
		if (!pool[VOLUMES].IsObject())
			return false;

		int migration = -1;
		if (pool[VOLUMES].MemberCount() <= 0)
			return false;
		else if (pool[VOLUMES].MemberCount() != 1) {
			if (!pool.HasMember(MIGRATION))
				return false;

			migration = pool[MIGRATION].GetInt();
		}

		for (auto &m: pool[VOLUMES].GetObject()) {
			bool validRules;
			switch (migration) {
				case Mtfs::TIME_MIGRATION:
					validRules = validateTime(m.value);
					break;
				case Mtfs::RIGHT_MIGRATION:
					validRules = validateRight(m.value);
					break;
				default:
					validRules = (pool[VOLUMES].MemberCount() == 1);
					break;
			}

			if (!validRules)
				return false;

			if (!validateVolume(m.value))
				return false;
		}

		return true;
	}

	bool ConfigFile::validateVolume(const Value &volume) {
		if (!volume.HasMember(TYPE))
			return false;

		PluginSystem::Plugin *plugin = getPlugin(volume[TYPE].GetString());
		if (plugin == NULL)
			return false;

		vector<string> infos = plugin->getInfos();
		infos.erase(infos.begin());
		for (auto i:infos)
			if (!volume.HasMember(i.c_str()))
				return false;

		return true;
	}


}  // namespace Configurator
