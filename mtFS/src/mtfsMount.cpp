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
#include <pluginSystem/PluginManager.h>
#include <mtfs/PoolManager.h>
#include <mtfs/Mtfs.h>
#include <mtfsFuse/MtfsFuse.h>
#include <utils/Fs.h>
#include <boost/filesystem.hpp>

#define HOME_DIR "/home/david/Cours/4eme/Travail_bachelor/Home"

using namespace std;
using namespace rapidjson;

int main(int argc, char **argv) {
	if (!Fs::dirExists(HOME_DIR)) {
		cerr << "Sorry no configured system found." << endl;
		cerr << "Please configure one or recover with -r device_in_system "
				"where device_in_system is a device wich was in the configuration." << endl;
		return -1;
	}

	string sysName = argv[argc - 1];
	string filename = sysName + ".json";
//	string filename = "home.json";
	string filepath = string(HOME_DIR) + "/" + mtfs::Mtfs::CONFIG_DIR + "/" + filename;
	argc--;

	if (!boost::filesystem::exists(filepath)) {
		cerr << "File not found" << endl;
		return -1;
	}

	chdir(HOME_DIR);
	ifstream file(string(mtfs::Mtfs::CONFIG_DIR) + "/" + filename);
	if (!file.is_open()) {
		cerr << "error openning file " << strerror(errno) << endl;
		return -1;
	}

	IStreamWrapper wrapper(file);
	Document d;
	d.ParseStream(wrapper);

//	validate config file.
	if (!mtfs::Mtfs::validate(d)) {
		cerr << "Invalid or corrupted JSON!" << endl;
		return -1;
	}

//	build mtfs
	mtfs::Mtfs::start(d, HOME_DIR, sysName);

//	build mtsfFuse.
	mtfsFuse::MtfsFuse *mtfsFuse = new mtfsFuse::MtfsFuse();

	int ret = mtfsFuse->run(argc, argv);

	mtfs::Mtfs::stop();

	return ret;
}

