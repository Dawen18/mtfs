/**
 * @author David Wittwer
 * @date 05.05.17.
**/

#include <iostream>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <Configurator/ConfigFile.h>
#include <Configurator/PluginManager.h>
#include <mtfs/PoolManager.h>
#include <mtfs/Mtfs.h>

#define HOME_DIR "/home/david/Cours/4eme/Travail_bachelor/Home"
#define STORAGE_DIR "StorageSystem"

using namespace std;
using namespace rapidjson;
using namespace Configurator;
//using namespace mtfs;

bool dirExists(string path);

bool fileExists(string dirPath, string filename);

int main(int argc, char **argv) {
	if (!dirExists(HOME_DIR)) {
		cerr << "Sorry no configured system found." << endl;
		cerr
				<< "Please configure one or recover with -o device_in_system "
						"where device_in_system is a device wich was in the configuration."
				<< endl;
		return -1;
	}

	string filename = string(argv[1]) + ".json";

	if (!fileExists(string(HOME_DIR) + "/" + STORAGE_DIR, filename)) {
		cerr << "File not found" << endl;
		return -1;
	}

	chdir(HOME_DIR);
	ifstream file(string(STORAGE_DIR) + "/" + filename);
	if (!file.is_open()) {
		cerr << "error openning file " << strerror(errno) << endl;
		return -1;
	}

	IStreamWrapper wrapper(file);
	Document d;
	d.ParseStream(wrapper);

	ConfigFile configFile(d);

	if (!configFile.validateFile()) {
		cerr << "Invalid or corrupted JSON!" << endl;
		return -1;
	}

	PluginManager *pluginManager = PluginManager::getInstance();

//	Instatiate classes


//	iter pools
	for (auto &p: d[ConfigFile::POOLS].GetObject()) {
		int poolId = stoi(p.name.GetString());
		cout << "poolId: " << poolId << endl;

		mtfs::Pool *pool = new mtfs::Pool();

		int migration = p.value[ConfigFile::MIGRATION].GetInt();

//		iter volumes
		for (auto &v: p.value.GetObject()[ConfigFile::VOLUMES].GetObject()) {
			Value vol(kObjectType);
			vol = v.value;

			int volumeId = stoi(v.name.GetString());
#ifdef DEBUG
			cout << "\tvolumeId: " << volumeId << " type: " << vol[ConfigFile::TYPE].GetString() << endl;
#endif

			PluginSystem::Plugin *plugin = pluginManager->getPlugin(vol[ConfigFile::TYPE].GetString());
			map<string, string> params;
			params["home"] = HOME_DIR + string("/Plugins");

//			Construct params
			for (auto &param: vol.GetObject()) {
				if (param.name.GetString() == "type")
					continue;

#ifdef DEBUG
				cout << "\t\t" << param.name.GetString() << endl;
#endif

				params[param.name.GetString()] = param.value.GetString();
			}

			if (!plugin->attach(params)) {
				cerr << "Failed to attach plugin " << plugin->getInfos()[0] << endl;
				delete (plugin);

				return -1;
			}

			mtfs::Volume *volume = new mtfs::Volume(plugin);

			pool->addVolume((uint32_t) stoi(v.name.GetString()), volume, mtfs::Mtfs::buildRule(migration, vol));
		}
	}

	return 0;
}


bool dirExists(string path) {
	struct stat info;

	if (stat(path.c_str(), &info) != 0)
		return false;
	else return (info.st_mode & S_IFDIR) != 0;
}

bool fileExists(string dirPath, string filename) {
	DIR *dp;
	dirent *d;

	if ((dp = opendir(dirPath.c_str())) == NULL)
		return false;

	while ((d = readdir(dp)) != NULL) {
		if (strcmp(d->d_name, filename.c_str()) == 0)
			return true;
	}
	return false;
}


