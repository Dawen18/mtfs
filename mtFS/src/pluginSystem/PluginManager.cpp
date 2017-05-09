/**
 * @author David Wittwer
 * @date 07.05.17.
**/

#include "pluginSystem/PluginManager.h"
#include "../../../Plugin/BlockDevice/BlockDevice.h"

using namespace std;

namespace pluginSystem {
	PluginManager *PluginManager::instance = 0;

	PluginManager *PluginManager::getInstance() {
		if (!instance)
			instance = new PluginManager();

		return instance;
	}

	pluginSystem::Plugin *PluginManager::getPlugin(string pluginName) {
		map<string, pluginSystem::Plugin *>::iterator it;

		it = pluginMap.find(pluginName);
		if (it == pluginMap.end())
			return nullptr;

		return new pluginSystem::BlockDevice;
	}

	PluginManager::PluginManager() {
		loadPlugins();
	}

	void PluginManager::loadPlugins() {
		pluginSystem::BlockDevice *blockDevice = new pluginSystem::BlockDevice();
		string name = blockDevice->getInfos()[0];

		pluginMap[name] = blockDevice;
	}
}
