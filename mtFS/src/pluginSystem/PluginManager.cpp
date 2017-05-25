/**
 * @author David Wittwer
 * @date 07.05.17.
**/
#include <dlfcn.h>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <pluginSystem/PluginManager.h>
#include <pluginSystem/Plugin.h>

#include "pluginSystem/PluginManager.h"

using namespace std;
using namespace boost::filesystem;

namespace pluginSystem {
	PluginManager *PluginManager::instance = 0;

	/******************************************************************************************************************/
	/*													Publics														  */
	/******************************************************************************************************************/

	PluginManager *PluginManager::getInstance() {
		if (!instance)
			instance = new PluginManager();

		return instance;
	}

	Plugin *PluginManager::getPlugin(string pluginName) {
		map<string, plugin_t>::iterator it;

		it = pluginMap.find(pluginName);
		if (it != pluginMap.end()) {
			this->lastError = PLUGIN_FOUND;
			return it->second.createObj();
		}

		if (!pluginExist(pluginName)) {
			this->lastError = PLUGIN_NOT_FOUND;
			return nullptr;
		}

		return loadPlugin(pluginName);
	}

	void PluginManager::freePlugin(std::string pluginName, Plugin *plugin) {
		if (this->pluginMap.find(pluginName) == this->pluginMap.end()) {
			this->lastError = PLUGIN_NOT_FOUND;
		}

		this->pluginMap[pluginName].destroyObj(plugin);
		this->lastError=SUCCESS;
	}

	int PluginManager::getError() {
		return this->lastError;
	}

	int PluginManager::getInfo(std::string pluginName, pluginInfo_t &info) {
		if (this->pluginMap.find(pluginName) == this->pluginMap.end()) {
			this->lastError = PLUGIN_NOT_FOUND;
			return PLUGIN_NOT_FOUND;
		}

		this->lastError = SUCCESS;
		info = this->pluginMap[pluginName].getInfo();

		return SUCCESS;
	}

	/******************************************************************************************************************/
	/*													Private														  */
	/******************************************************************************************************************/

	PluginManager::PluginManager() {
	}

	bool PluginManager::pluginExist(std::string name) {
		string dir("./");
		dir += PLUGIN_DIR;

		if (is_directory(dir)) {
			for (auto &entry  :boost::make_iterator_range(directory_iterator(dir))) {
				if (entry.path().filename() == (string("lib") + name + ".so")) {
					return true;
				}
			}
		}
		return false;
	}

	Plugin *PluginManager::loadPlugin(string name) {
		plugin_t plugin;
		string path = string("./") + PLUGIN_DIR + "/lib" + name + ".so";

		void *library = dlopen(path.c_str(), RTLD_LAZY);
		if (!library) {
			cerr << "Cannot loab library: " << dlerror() << endl;
			return nullptr;
		}
		dlerror();

		plugin.createObj = (pluginSystem::Plugin *(*)()) dlsym(library, "createObj");
		char *dlsym_error = dlerror();
		if (dlsym_error) {
			cerr << "Cannot load symbol create: " << dlsym_error << endl;
			return nullptr;
		}
		dlerror();

		plugin.destroyObj = (void (*)(Plugin *)) dlsym(library, "destroyObj");
		dlsym_error = dlerror();
		if (dlsym_error) {
			cerr << "Cannot load symbol destroy: " << dlsym_error << endl;
			return nullptr;
		}
		dlerror();

		plugin.getInfo = (pluginInfo_t (*)()) dlsym(library, "getInfo");
		dlsym_error = dlerror();
		if (dlsym_error) {
			cerr << "Cannot load symbol info: " << dlsym_error << endl;
			return nullptr;
		}
		dlerror();

		dlclose(library);

		this->pluginMap.insert(make_pair(plugin.getInfo().name, plugin));

		return plugin.createObj();
	}
}
