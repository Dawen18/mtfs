/**
 * \file mtfsCreate.cpp
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

//#include <mtfs/structs.h>
#include <option/optionparser.h>
#include <iostream>
#include <utils/Fs.h>
#include <rapidjson/document.h>
#include <mtfs/Mtfs.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <fstream>
#include <rapidjson/istreamwrapper.h>
#include <mtfs/TimeRule.h>
#include <mtfs/PoolManager.h>
#include <algorithm>
#include <zconf.h>
#include <boost/filesystem.hpp>
#include <pluginSystem/PluginManager.h>

using namespace std;
using namespace mtfs;
using namespace rapidjson;

struct Arg : public option::Arg {
	static void printError(const char *msg1, const option::Option &opt, const char *msg2) {
		fprintf(stderr, "ERROR: %s", msg1);
		fwrite(opt.name, (size_t) opt.namelen, 1, stderr);
		fprintf(stderr, "%s", msg2);
	}

	static option::ArgStatus Numeric(const option::Option &option, bool msg) {
		char *endptr = nullptr;
		if (option.arg != nullptr && (strtol(option.arg, &endptr, 10) != 0)) {};
		if (endptr != option.arg && *endptr == 0)
			return option::ARG_OK;

		if (msg) printError("Option '", option, "' requires a numeric argument\n");
		return option::ARG_ILLEGAL;
	}

	static option::ArgStatus NonEmpty(const option::Option &option, bool msg) {
		if (option.arg != nullptr && option.arg[0] != 0)
			return option::ARG_OK;

		if (msg) printError("Option '", option, "' requires a non-empty argument\n");
		return option::ARG_ILLEGAL;
	}

	static option::ArgStatus Migration(const option::Option &option, bool msg) {
		return option::ARG_OK;
	}
};

enum optionIndex {
	UNKNOWN,
	NEW,
	ICACHE,
	DCACHE,
	BCACHE,
	BSIZE,
	REDUNDANCY,
	MIGRATION,
	POOL,
	VOLUME,
	ADD,
	DEL,
	CONFIG,
	INSTALL,
	HELP
};
const option::Descriptor usage[] =
		{
				{UNKNOWN,    0, "",      "",            Arg::None,         "USAGE: mtfsCreate [options] <system_name>"},
				{NEW,        0, "n",     "new",         Arg::NonEmpty,     "  -n, \t--new    \tCreate a new storage."},
				{ICACHE,     0, "i",     "inode-cache", Arg::Numeric,      "  -i[SIZE], \t--inode-cache[=SIZE]  \tSet inode cache size."},
				{DCACHE,     0, "d",     "dir-cache",   Arg::Numeric,      "  -d[SIZE], \t--dir-cache[=SIZE]  \tSet directory entry cache size."},
				{BCACHE,     0, "b",     "block-cache", Arg::Numeric,      "  -b[SIZE], \t--block-cache[=SIZE]  \tSet block cache size."},
				{BSIZE,      0, "s",     "block-size",  Arg::Numeric,      "  -s[SIZE], \t--block-size[=SIZE]  \tSet block size. Default 4096 octets"},
				{REDUNDANCY, 0, "r",     "redundancy",  Arg::Numeric,      "  -r[NB], \t--redundancy[=NB]  \tSet number of redundancy for each block or inode."},
				{MIGRATION,  0, "m",     "migration",   Arg::Migration,    "  -m[TYPE], \t--migration[=TYPE]  \tSet migration type in system or pool if -p is used"},
				{POOL,       0, "p",     "pool",        Arg::Numeric,      "  -p[ID], \t--pool[=ID]    \tSpecify pool."},
				{VOLUME,     0, "v",     "volume",      Arg::Numeric,      "  -v[ID], \t--volume[=ID]    \tSpecify volume in pool."},
				{ADD,        0, "",      "add",         Arg::None,         "  \t--add  \tAdd a pool or volume."},
				{DEL,        0, "",      "del",         Arg::None,         "  \t--del  \tDelete a pool or volume."},
				{CONFIG,     0, "c",     "config",      Arg::NonEmpty,     "  -c[CONFIG], \t--config[=CONFIG]  \tSet config for plugin or migration (json string or filename which contains de json config."},
				{INSTALL,    0, "",      "install",     Arg::None,         "  -i,  \t--install  \tInstall the config."},
				{HELP,       0, "h",     "help",        option::Arg::None, "  -h, \t--help \tPrint this help and exit."},
				{UNKNOWN,    0, "",      "",            option::Arg::None, "\nExamples:\n"
																				   " Create system:\n"
																				   "  mtfsCreate -n mtfsRoot\n"
																				   " Add pool:\n"
																				   "  mtfsCreate -p mtfsRoot\n"
																				   " Add volume in pool:\n"
																				   "  mtfsCreate -v mtfsRoot 1\n"},
				{0,          0, nullptr, nullptr,       nullptr,           nullptr}
		};

bool configExist(const string &name);

void loadConfig(superblock_t &sb, const string &name);

bool writeConfig(superblock_t &superblock, const string &confName);

uint32_t addVolume(pool_t &pool, volume_t &volume);

uint32_t addPool(superblock_t &sb, pool_t &pool);

uint32_t findMissing(std::vector<uint32_t> &x, uint32_t number);

void installConfig(superblock_t &superblock, const string &name);


int main(int argc, char **argv) {
//	std::cout << MTFS_CONFIG_DIR << std::endl;
	argc -= (argc > 0);
	argv += (argc > 0);
	option::Stats stats(usage, argc, argv);
	option::Option options[stats.options_max], buffer[stats.buffer_max];
	option::Parser parse(usage, argc, argv, options, buffer);

	if (parse.error())
		return 1;

	if ((nullptr != options[HELP]) || 0 == argc) {
		option::printUsage(cerr, usage);
		return 0;
	}

	for (option::Option *opt = options[UNKNOWN]; opt; opt = opt->next())
		cerr << "Unknown option: " << opt->name << "\n";
	if (options[UNKNOWN] != nullptr)
		return -1;

#ifdef DEBUG
	for (int i = 0; i < parse.nonOptionsCount(); ++i)
		std::cout << "Non-option #" << i << ": " << parse.nonOption(i) << "\n";
#endif

	if (0 != chdir(MTFS_HOME_DIR)) {
		return errno;
	}

	superblock_t superblock;
	memset(&superblock, 0, sizeof(superblock_t));
	superblock.pools.clear();

	string confName;

//	if option new.
	if (options[NEW] != nullptr) {
#ifndef DEBUG
		if (configExist(options[NEW].arg)) {
			cerr << "Config '" << options[NEW].arg << "' already exist" << endl;
			return -1;
		}
#endif

		confName = options[NEW].arg;

		superblock.iCacheSz = superblock.bCacheSz = superblock.dCacheSz = superblock.blockSz = 4096;
		superblock.redundancy = 1;
		superblock.migration = Rule::TIME_MIGRATION;

	} else {
		confName = parse.nonOption(0);
		if (!configExist(confName)) {
			cerr << "Config not exist" << endl;
			return -1;
		}

#ifdef DEBUG
		cout << "Load config " << confName << endl;
#endif

		loadConfig(superblock, confName);
	}

	if (nullptr != options[ICACHE]) {
		superblock.iCacheSz = static_cast<size_t>(stoi(options[ICACHE].arg));
	}

	if (nullptr != options[DCACHE]) {
		superblock.dCacheSz = static_cast<size_t>(stoi(options[DCACHE].arg));
	}

	if (nullptr != options[BCACHE]) {
		superblock.bCacheSz = static_cast<size_t>(stoi(options[BCACHE].arg));
	}

	if (nullptr != options[BSIZE]) {
		superblock.blockSz = static_cast<size_t>(stoi(options[BSIZE].arg));
	}

	if (nullptr != options[REDUNDANCY]) {
		superblock.redundancy = static_cast<size_t>(stoi(options[REDUNDANCY].arg));
	}

	if (nullptr != options[MIGRATION] && nullptr == options[POOL]) {
		string migration = options[MIGRATION].arg;

		if ("time" == migration) {
			superblock.migration = Rule::TIME_MIGRATION;
		} else if ("user" == migration) {
			superblock.migration = Rule::RIGHT_MIGRATION;
		} else {
			cerr << "unknow migration";
			return -1;
		}
	} else if (nullptr != options[MIGRATION] && nullptr != options[POOL]) {
	}

	if (nullptr != options[ADD] && nullptr == options[POOL]) {
//		ADD one pool
#ifdef DEBUG
		cout << "add a pool" << endl;
#endif
		pool_t pool;
		pool.migration = Rule::TIME_MIGRATION;
		pool.volumes.clear();

		if (nullptr != options[CONFIG]) {
			string arg = options[CONFIG].arg;
			Document tmpDoc;
#ifdef DEBUG
			cout << "pool arg: " << arg << endl;
#endif

			if (".json" == arg.substr(arg.length() - 5)) {
//				TODO Parse json file
			} else
				tmpDoc.Parse(arg.c_str());

			if (0 > Rule::rulesAreValid(superblock.migration, tmpDoc)) {
				cerr << "Invalid config!" << endl;
				return -1;
			}

			if (tmpDoc.HasMember(Rule::MIGRATION))
				pool.migration = tmpDoc[Rule::MIGRATION].GetInt();

			if (tmpDoc.HasMember(TimeRule::TIME_LOW_LIMIT)) {
				tmpDoc[TimeRule::TIME_LOW_LIMIT].SetUint(tmpDoc[TimeRule::TIME_LOW_LIMIT].GetUint() * 60);
			}

			if (tmpDoc.HasMember(TimeRule::TIME_HIGH_LIMIT))
				tmpDoc[TimeRule::TIME_HIGH_LIMIT].SetUint(tmpDoc[TimeRule::TIME_HIGH_LIMIT].GetUint() * 60);

			pool.rule = Rule::buildRule(superblock.migration, tmpDoc);
		} else {
//			cerr << "config needed!" << endl;
//			return -1;
			Document d;
			d.SetObject();
			Value v;
			v.SetInt(0);
			d.AddMember(StringRef("timeHighLimit"), v, d.GetAllocator());
			pool.rule = Rule::buildRule(superblock.migration, d);
		}

		int newPoolId = addPool(superblock, pool);
		if (0 < newPoolId)
			cout << "new pool:" << newPoolId << endl;

	} else if (nullptr != options[ADD] && nullptr != options[POOL]) {
//		ADD one volume.
		if (superblock.pools.end() == superblock.pools.find(stoul(options[POOL].arg))) {
			cerr << "pool '" << options[POOL].arg << "' not found" << endl;
			return -1;
		}

		uint32_t poolId = (uint32_t) stoul(options[POOL].arg);
		volume_t volume;
		pool_t *pool = &superblock.pools[poolId];

		if (nullptr != options[CONFIG]) {
			string arg = options[CONFIG].arg;
			Document tmpDoc;
#ifdef DEBUG
			cout << "pool arg: " << arg << endl;
#endif

			if (".json" == arg.substr(arg.length() - 5)) {
//				TODO Parse json file
			} else
				tmpDoc.Parse(arg.c_str());

			if (0 > Rule::rulesAreValid(pool->migration, tmpDoc)) {
				cerr << "Invalid config!" << endl;
				return -1;
			}

			if (!tmpDoc.HasMember(pluginSystem::Plugin::TYPE)) {
				cerr << R"(config need a plugin name eg: {"plName":"block"})" << endl;
				return -1;
			}
			volume.pluginName = tmpDoc[pluginSystem::Plugin::TYPE].GetString();

			if (tmpDoc.HasMember(pluginSystem::Plugin::PARAMS)) {
				for (auto &&param: tmpDoc[pluginSystem::Plugin::PARAMS].GetObject()) {
					volume.params.insert(make_pair(param.name.GetString(), param.value.GetString()));
				}
			}

			if (tmpDoc.HasMember(TimeRule::TIME_LOW_LIMIT)) {
				tmpDoc[TimeRule::TIME_LOW_LIMIT].SetUint(tmpDoc[TimeRule::TIME_LOW_LIMIT].GetUint() * 60);
			}

			if (tmpDoc.HasMember(TimeRule::TIME_HIGH_LIMIT))
				tmpDoc[TimeRule::TIME_HIGH_LIMIT].SetUint(tmpDoc[TimeRule::TIME_HIGH_LIMIT].GetUint() * 60);


			volume.rule = Rule::buildRule(pool->migration, tmpDoc);
		} else {
			cerr << "config needed!" << endl;
			return -1;
		}

		int newVolumeId = addVolume(*pool, volume);
		if (0 < newVolumeId)
			cout << "new volume:" << newVolumeId << endl;
	}

//	TODO catch -p, -v anc -c options

	if (nullptr != options[INSTALL]) {
		installConfig(superblock, confName);
		return EXIT_SUCCESS;
	}

	writeConfig(superblock, confName);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////Definitions//////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool configExist(const string &name) {
	return Fs::fileExists(MTFS_CONFIG_DIR, name + ".json");
}

void loadConfig(superblock_t &sb, const string &name) {
	string filename = string(MTFS_CONFIG_DIR) + name + ".json";
	ifstream file(filename);
	if (!file.is_open()) {
		return;
	}

	IStreamWrapper wrapper(file);
	Document confFile;

	confFile.ParseStream(wrapper);

	Mtfs::jsonToStruct(confFile, sb);
}

bool writeConfig(superblock_t &superblock, const string &confName) {
	Document d;
	Document::AllocatorType &allocator = d.GetAllocator();
	d.SetObject();

	Value v;

	Mtfs::structToJson(superblock, d);

	StringBuffer sb;
	PrettyWriter<StringBuffer> pw(sb);
	d.Accept(pw);

	string filename = string(MTFS_CONFIG_DIR) + confName + ".json";
	ofstream configFile(filename);
	configFile << sb.GetString() << endl;
	configFile.close();

	return true;
}

/**
 * @brief Add volume in pool
 * 
 * @param pool 
 * @param volume 
 * 
 * @return The volume Id
 */
uint32_t addVolume(pool_t &pool, volume_t &volume) {
	uint32_t volumeId = 0;
	vector<uint32_t> ids;
	ids.clear();

	for (auto &item  : pool.volumes) {
		ids.push_back(item.first);
	}
	if (ids.empty())
		volumeId = 1;
	else
		volumeId = findMissing(ids, 0);

	if (pool.volumes.end() != pool.volumes.find(volumeId))
		return 0;

	pool.volumes.insert(make_pair(volumeId, volume));

	return volumeId;
}

/**
 * @brief add a pool in system
 *
 * @param sb 	The superblock
 * @param pool 	The pool to add
 *
 * @return the pool id
 */
uint32_t addPool(superblock_t &sb, pool_t &pool) {
	uint32_t poolId;
	vector<uint32_t> ids;
	ids.clear();

	for (auto &item  : sb.pools) {
		ids.push_back(item.first);
	}
	if (ids.empty())
		poolId = 1;
	else
		poolId = findMissing(ids, 0);

	if (sb.pools.end() != sb.pools.find(poolId))
		return 0;

	sb.pools.insert(make_pair(poolId, pool));

	return poolId;
}

/**
 * Function find on stackOverflow
 *
 * http://stackoverflow.com/questions/28176191/find-first-missing-element-in-a-vector
 *
 * @param x vector with content
 * @param number Number from which to search
 * @return The missing number.
 */
uint32_t findMissing(std::vector<uint32_t> &x, uint32_t number) {
	std::sort(x.begin(), x.end());
	auto pos = std::upper_bound(x.begin(), x.end(), number);

	if (*pos - number > 1)
		return number + 1;

	std::vector<int> diffs;
	std::adjacent_difference(pos, x.end(), std::back_inserter(diffs));
	auto pos2 = std::find_if(diffs.begin() + 1, diffs.end(), [](int x) { return x > 1; });
	return *(pos + (pos2 - diffs.begin() - 1)) + 1;

}

void installConfig(superblock_t &superblock, const string &name) {
//	create ident for rootInode.
	superblock.rootInodes.clear();
	const int rootRedundancy = max(3, (int) superblock.redundancy);
	int i = 0;
	for (auto &&pool: superblock.pools) {
		for (auto &&volume: pool.second.volumes) {
			if (rootRedundancy > i)
				superblock.rootInodes.emplace_back(0, volume.first, pool.first);
			else
				break;
			i++;
		}
	}

	writeConfig(superblock, name);

	string filename = "superblock.json";
	boost::filesystem::create_directory(MTFS_INSTALL_DIR + name);
	string src = string(MTFS_CONFIG_DIR) + name + ".json";
	string dst = string(MTFS_INSTALL_DIR) + name + "/" + filename;
	boost::filesystem::copy_file(src, dst, boost::filesystem::copy_option::overwrite_if_exists);
	string rootFilename = string(MTFS_INSTALL_DIR) + name + "/root.json";

	inode_t rootInode;
	Mtfs::createRootInode(rootInode);

	Document rootJSON(kObjectType);
	rootInode.toJson(rootJSON);

//	Attach all plugins for write superblock and rootInode
//	Only 3 firsts for rootInode
	pluginSystem::PluginManager *manager = pluginSystem::PluginManager::getInstance();
	i = 0;
	for (auto &&pool: superblock.pools) {
		for (auto &&volume: pool.second.volumes) {
			volume.second.params["home"] = MTFS_PLUGIN_HOME;
			volume.second.params["blockSize"] = to_string(superblock.blockSz);

			pluginSystem::Plugin *plugin;

			plugin = manager->getPlugin(volume.second.pluginName);
			plugin->attach(volume.second.params);

			if (rootRedundancy > i)
				plugin->put(0, &rootInode, INODE, false);
			plugin->putSuperblock(superblock);

			i++;

			plugin->detach();
		}
	}

	StringBuffer sb;
	PrettyWriter<StringBuffer> pw(sb);
	rootJSON.Accept(pw);

	ofstream configFile(rootFilename);
	configFile << sb.GetString() << endl;
	configFile.close();
}

