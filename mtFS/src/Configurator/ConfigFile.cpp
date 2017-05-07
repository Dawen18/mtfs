
#include <iostream>
#include "Configurator/ConfigFile.h"
#include "../../../Plugin/BlockDevice/BlockDevice.h"
#include <mtfs/Mtfs.h>

namespace Configurator {
	using namespace std;
	using namespace rapidjson;
	using namespace mtfs;

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
			throw invalid_argument("Not a object!");

		if (!d.HasMember(INODE_CACHE))
			throw invalid_argument("Inode cache missing!");

		if (!d.HasMember(DIR_CACHE))
			throw invalid_argument("Directory cache missing!");

		if (!d.HasMember(BLOCK_CACHE))
			throw invalid_argument("Block cache missing!");

		if (!d.HasMember(BLOCK_SIZE))
			throw invalid_argument("Block size missing!");

		if (!d.HasMember(REDUNDANCY))
			throw invalid_argument("Redundancy missing!");

		if (!d.HasMember(POOLS))
			throw invalid_argument("Pools missing!");
		if (!d[POOLS].IsObject())
			throw invalid_argument("Pool is not a object!");

		int migration = -1;
		if (d[POOLS].MemberCount() <= 0)
			throw invalid_argument("Number of pool invalid!");
		else if (d[POOLS].MemberCount() != 1) {
			if (!d.HasMember(MIGRATION))
				throw invalid_argument("Migration missing!");

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
				throw invalid_argument(string("Rules invalid for pool '") + m.name.GetString() + "'");

			if (!validatePool(m.value))
				throw invalid_argument(string("Pool '") + m.name.GetString() + "' invalid!");
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
			throw invalid_argument("Volumes missing!");
		if (!pool[VOLUMES].IsObject())
			throw invalid_argument("Volue is not a object");

		int migration = -1;
		if (pool[VOLUMES].MemberCount() <= 0)
			throw invalid_argument("Number of volumes invalid!");
		else if (pool[VOLUMES].MemberCount() != 1) {
			if (!pool.HasMember(MIGRATION))
				throw invalid_argument("Migration missing!");

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
				throw invalid_argument(string("Rules invalid for volume '") + m.name.GetString() + "'!");

			if (!validateVolume(m.value))
				throw invalid_argument(string("Volume '")+m.name.GetString()+"' invalid!");
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
