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

#define HOME_DIR "/home/david/Cours/4eme/Travail_bachelor/Home"
#define STORAGE_DIR "StorageSystem"

using namespace std;
using namespace rapidjson;

bool dirExists(string path);

bool fileExists(string dirPath, string filename);

int main(int argc, char **argv) {
	if (!dirExists(HOME_DIR)) {
		cerr << "Sorry no configured system found." << endl;
		cerr
				<< "Please configure one or recover with -r device_in_system "
						"where device_in_system is a device wich was in the configuration."
				<< endl;
		return -1;
	}

//	string filename = string(argv[2]) + ".json";
	string filename = "home.json";

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

//	validate config file.
	if (!mtfs::Mtfs::validate(d)) {
		cerr << "Invalid or corrupted JSON!" << endl;
		return -1;
	}

	ThreadQueue<string> *queue = new ThreadQueue<string>();

//	build mtfs
	mtfs::Mtfs *mtfs = mtfs::Mtfs::getInstance();
	mtfs->build(d, HOME_DIR);
	mtfs->setSynchronousQueue(queue);
	mtfs->start();

//	build mtsfFuse.
	mtfsFuse::MtfsFuse *mtfsFuse = new mtfsFuse::MtfsFuse();
	mtfsFuse->setThreadQueue(queue);

	return mtfsFuse->run(argc, argv);

//	return 0;
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


