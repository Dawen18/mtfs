/**
 * @author David Wittwer
 * @date 07.05.17.
**/

#include "Configurator/PluginManager.h"
#include "../../../Plugin/BlockDevice/BlockDevice.h"

using namespace std;

PluginManager *PluginManager::instance = 0;

PluginManager *PluginManager::getInstance() {
	if (!instance)
		instance = new PluginManager();

	return instance;
}

PluginSystem::Plugin *PluginManager::getPlugin(string pluginName) {
	map<string, PluginSystem::Plugin *>::iterator it;

	it = pluginMap.find(pluginName);
	if (it == pluginMap.end())
		return nullptr;

	return new PluginSystem::BlockDevice;
}

PluginManager::PluginManager() {
	loadPlugins();
}

void PluginManager::loadPlugins() {
	PluginSystem::BlockDevice *blockDevice = new PluginSystem::BlockDevice();
	string name = blockDevice->getInfos()[0];

	pluginMap[name] = blockDevice;
}
