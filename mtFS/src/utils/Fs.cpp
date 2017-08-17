/**
 * \file Fs.cpp
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
