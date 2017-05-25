#include <dlfcn.h>
#include <iostream>
#include <bits/unique_ptr.h>
#include <pluginSystem/Plugin.h>
#include <zconf.h>
#include <pluginSystem/PluginManager.h>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>

/**
 * @author David Wittwer
 * @date 21.05.17.
**/

#define HOME "/home/david/Cours/4eme/Travail_bachelor/Home"
#define BLOCK_SIZE 4096

using namespace std;
using namespace boost::filesystem;

int main(int argc, char **argv) {
	chdir(HOME);

	pluginSystem::PluginManager *manager = pluginSystem::PluginManager::getInstance();
	pluginSystem::Plugin *plugin;
	plugin = manager->getPlugin("block");
	map<string, string> params;
	string home(HOME);
	params.insert(make_pair("home", home + "/Plugins"));
	params.insert(make_pair("blockSize", to_string(BLOCK_SIZE)));
	params.insert(make_pair("devicePath", "/dev/sdd2"));
	params.insert(make_pair("fsType", "ext4"));
	if (!plugin->attach(params))
		cout << "attach fail" << endl;
	else
		cout << "attach success" << endl;
	plugin->detach();

	params.clear();
	manager->freePlugin("block", plugin);
}

