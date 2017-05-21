/**
 * @author David Wittwer
 * @date 10.05.17.
**/

#include <utils/Fs.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>

bool Fs::fileExists(std::string dirPath, std::string filename) {
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

bool Fs::dirExists(std::string path) {
	struct stat info;

	if (stat(path.c_str(), &info) != 0)
		return false;
	else return (info.st_mode & S_IFDIR) != 0;
}
