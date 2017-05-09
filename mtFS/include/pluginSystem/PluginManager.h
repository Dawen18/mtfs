/**
 * @author David Wittwer
 * @date 07.05.17.
**/

#ifndef TRAVAIL_BACHELOR_PLUGINMANAGER_H
#define TRAVAIL_BACHELOR_PLUGINMANAGER_H


#include <map>
#include <pluginSystem/Plugin.h>

namespace pluginSystem {
	class PluginManager {
	public:
		static PluginManager *getInstance();

		PluginSystem::Plugin *getPlugin(std::string pluginName);

	private:
		PluginManager();

		void loadPlugins();


		std::map<std::string, PluginSystem::Plugin *> pluginMap;
		static PluginManager *instance;

	};
}


#endif //TRAVAIL_BACHELOR_PLUGINMANAGER_H
