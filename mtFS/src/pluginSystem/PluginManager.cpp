/**
 * \file PluginManager.cpp
 * \brief
 * \author David Wittwer
 * \version 0.0.1
 * \copyright GNU Publis License V3
 *
 * This file is part of MTFS.

    MTFS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <dlfcn.h>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <pluginSystem/PluginManager.h>
#include <pluginSystem/Plugin.h>

#include "pluginSystem/PluginManager.h"
#include "../../../Plugin/S3/S3.h"

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
			return;
		}

		this->pluginMap[pluginName].destroyObj(plugin);
		this->lastError = SUCCESS;
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
		plugin_t plugin{};
		string path = string("./") + PLUGIN_DIR + "/lib" + name + ".so";

//		Open plugin
		void *library = dlopen(path.c_str(), RTLD_LAZY);
		if (nullptr == library) {
			cerr << "Cannot load plugin '" << name << "': " << dlerror() << endl;
			return nullptr;
		}
		dlerror();

//		Load createObj symbol
		plugin.createObj = (pluginSystem::Plugin *(*)()) dlsym(library, "createObj");
		char *dlsym_error = dlerror();
		if (nullptr != dlsym_error) {
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
