#ifndef CONFIGURATOR_CONFIG_FILE_H
#define CONFIGURATOR_CONFIG_FILE_H

#include <string>
#include <rapidjson/document.h>
#include <vector>
#include <PluginSystem/Plugin.h>


namespace Configurator {
	class ConfigFile {
	public:
		static constexpr const char *INODE_CACHE = "inodeCacheSize";
		static constexpr const char *DIR_CACHE = "directoryCacheSize";
		static constexpr const char *BLOCK_CACHE = "blockCacheSize";
		static constexpr const char *BLOCK_SIZE = "blockSize";
		static constexpr const char *REDUNDANCY = "redundancy";
		static constexpr const char *MIGRATION = "migration";
		static constexpr const char *POOLS = "pools";
		static constexpr const char *VOLUMES = "volumes";
		static constexpr const char *TIME_LOW_LIMIT = "lowLimit";
		static constexpr const char *TIME_HIGH_LIMIT = "highLimit";
		static constexpr const char *ALLOW_USER = "allowUsers";
		static constexpr const char *DENY_USER = "denyUsers";
		static constexpr const char *ALLOW_GROUP = "allowGroups";
		static constexpr const char *DENY_GROUP = "denyGroups";
		static constexpr const char *TYPE = "type";

	private:
		const rapidjson::Document &d;

		std::vector<PluginSystem::Plugin *> plugins;

	public:
		ConfigFile(const rapidjson::Document &d);

		bool validateFile();

		bool validateRight(const rapidjson::Value &value);

		bool validateTime(const rapidjson::Value &value);

		bool validatePool(const rapidjson::Value &pool);

		bool validateVolume(const rapidjson::Value &volume);

	private:
		void loadPlugins();

		PluginSystem::Plugin *getPlugin(std::string name);

	};

}  // namespace Configurator
#endif
