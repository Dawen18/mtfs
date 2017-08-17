/**
 * \file mtfsMount.cpp
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

#define FUSE_USE_VERSION 30

#include <iostream>
#include <unistd.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <pluginSystem/PluginManager.h>
#include <mtfs/PoolManager.h>
#include <mtfs/Mtfs.h>
#include <wrapper/MtfsFuse.h>
#include <utils/Fs.h>
#include <boost/filesystem.hpp>
#include <utils/Logger.h>


using namespace std;
using namespace rapidjson;

int main(int argc, char **argv) {
	if (!Fs::dirExists(MTFS_HOME_DIR)) {
		cerr << "Sorry no configured system found." << endl;
		cerr << "Please configure one or recover with -r device_in_system "
				"where device_in_system is a device wich was in the configuration." << endl;
		return -1;
	}


	string sysName = argv[argc - 1];
	string filename = sysName + ".json";
	string filepath = string(MTFS_HOME_DIR) + "/" + mtfs::Mtfs::CONFIG_DIR + "/" + filename;
	argc--;

	if (!boost::filesystem::exists(filepath)) {
		cerr << "File not found" << endl;
		return -1;
	}

	chdir(MTFS_HOME_DIR);
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
	mtfs::Mtfs::start(d, MTFS_HOME_DIR, sysName);

//	build mtsfFuse.
	wrapper::MtfsFuse *mtfsFuse;
	mtfsFuse = new wrapper::MtfsFuse();

	int ret = mtfsFuse->run(argc, argv);

	mtfs::Mtfs::stop();

	return ret;
}

