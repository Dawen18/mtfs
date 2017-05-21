/**
 * @author David Wittwer
 * @date 10.05.17.
**/

#ifndef MTFS_FS_H
#define MTFS_FS_H

#include <iosfwd>
#include <string>

class Fs {
public:
	static bool fileExists(std::string dirPath, std::string filename);

	static bool dirExists(std::string path);
};

#endif //MTFS_FS_H
