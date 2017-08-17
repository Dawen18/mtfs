/**
 * \file PluginManager.h
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

#ifndef TRAVAIL_BACHELOR_PLUGINMANAGER_H
#define TRAVAIL_BACHELOR_PLUGINMANAGER_H

#include <map>
#include <pluginSystem/Plugin.h>

namespace pluginSystem {
	typedef struct {
		pluginSystem::Plugin *(*createObj)();

		void (*destroyObj)(Plugin *plugin);

		pluginInfo_t (*getInfo)();
	} plugin_t;

	class PluginManager {
	public:
		static const int SUCCESS = 0;
		static const int PLUGIN_FOUND = 0;
		static const int PLUGIN_NOT_FOUND = -1;

		/**
		 * @brief Get manager instance
		 *
		 * @return Instance of PluginManager
		 */
		static PluginManager *getInstance();

		/**
		 * @brief Get plugin
		 *
		 * @param pluginName Name of plugin
		 * @return nullptr if no plugin found
		 */
		Plugin *getPlugin(std::string pluginName);

		/**
		 * @brief destroy plugin object
		 *
		 * @param pluginName Name of plugin.
		 * @param plugin Tthe object to destroy.
		 */
		void freePlugin(std::string pluginName, Plugin *plugin);

		/**
		 * @brief Get plugin info.
		 *
		 * @param pluginName Name of plugin.
		 * @param info Struct who contains the info @see pluginInfo_t.
		 * @return SUCCESS or PLUGIN_NOT_FOUND
		 */
		int getInfo(std::string pluginName, pluginInfo_t &info);

		/**
		 * @brief Get last error code.
		 *
		 * @return The error code.
		 */
		int getError();

	private:
		static constexpr const char *PLUGIN_DIR = "Libs";

		std::map<std::string, plugin_t> pluginMap;
		static PluginManager *instance;
		int lastError;


		PluginManager();

		bool pluginExist(std::string name);

		Plugin *loadPlugin(std::string name);

	};
}


#endif //TRAVAIL_BACHELOR_PLUGINMANAGER_H
