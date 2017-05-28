/**
 * @author David Wittwer
 * @date 30.04.17.
**/
#define FUSE_USE_VERSION 30
#define HOME_DIR "/home/david/Cours/4eme/Travail_bachelor/Home/"
#define PLUGIN_HOME "/home/david/Cours/4eme/Travail_bachelor/Home/Plugins/"
#define CONF_DIR "/home/david/Cours/4eme/Travail_bachelor/Home/Configs/"
#define INSTALL_DIR "/home/david/Cours/4eme/Travail_bachelor/Home/Systems/"

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
		char *endptr = 0;
		if (option.arg != 0 && strtol(option.arg, &endptr, 10)) {};
		if (endptr != option.arg && *endptr == 0)
			return option::ARG_OK;

		if (msg) printError("Option '", option, "' requires a numeric argument\n");
		return option::ARG_ILLEGAL;
	}

	static option::ArgStatus NonEmpty(const option::Option &option, bool msg) {
		if (option.arg != 0 && option.arg[0] != 0)
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
				{UNKNOWN,    0, "",  "",            Arg::None,         "USAGE: mtfsCreate [options] <system_name>"},
				{NEW,        0, "n", "new",         Arg::NonEmpty,     "  -n, \t--new    \tCreate a new storage."},
				{ICACHE,     0, "i", "inode-cache", Arg::Numeric,      "  -i[SIZE], \t--inode-cache[=SIZE]  \tSet inode cache size."},
				{DCACHE,     0, "d", "dir-cache",   Arg::Numeric,      "  -d[SIZE], \t--dir-cache[=SIZE]  \tSet directory entry cache size."},
				{BCACHE,     0, "b", "block-cache", Arg::Numeric,      "  -b[SIZE], \t--block-cache[=SIZE]  \tSet block cache size."},
				{BSIZE,      0, "s", "block-size",  Arg::Numeric,      "  -s[SIZE], \t--block-size[=SIZE]  \tSet block size. Default 4096 octets"},
				{REDUNDANCY, 0, "r", "redundancy",  Arg::Numeric,      "  -r[NB], \t--redundancy[=NB]  \tSet number of redundancy for each block or inode."},
				{MIGRATION,  0, "m", "migration",   Arg::Migration,    "  -m[TYPE], \t--migration[=TYPE]  \tSet migration type in system or pool if -p is used"},
				{POOL,       0, "p", "pool",        Arg::Numeric,      "  -p[ID], \t--pool[=ID]    \tSpecify pool."},
				{VOLUME,     0, "v", "volume",      Arg::Numeric,      "  -v[ID], \t--volume[=ID]    \tSpecify volume in pool."},
				{ADD,        0, "",  "add",         Arg::None,         "  \t--add  \tAdd a pool or volume."},
				{DEL,        0, "",  "del",         Arg::None,         "  \t--del  \tDelete a pool or volume."},
				{CONFIG,     0, "c", "config",      Arg::NonEmpty,     "  -c[CONFIG], \t--config[=CONFIG]  \tSet config for plugin or migration (json string or filename which contains de json config."},
				{INSTALL,    0, "",  "install",     Arg::None,         "  -i,  \t--install  \tInstall the config."},
				{HELP,       0, "h", "help",        option::Arg::None, "  -h, \t--help \tPrint this help and exit."},
				{UNKNOWN,    0, "",  "",            option::Arg::None, "\nExamples:\n"
																			   " Create system:\n"
																			   "  mtfsCreate -n mtfsRoot\n"
																			   " Add pool:\n"
																			   "  mtfsCreate -p mtfsRoot\n"
																			   " Add volume in pool:\n"
																			   "  mtfsCreate -v mtfsRoot 1\n"},
				{0,          0, 0,   0,             0,                 0}
		};

bool configExist(string name);

void loadConfig(superblock_t &sb, string name);

bool writeConfig(superblock_t &superblock, string confName);

uint32_t addVolume(pool_t &pool, volume_t &volume);

uint32_t addPool(superblock_t &sb, pool_t &pool);

uint32_t findMissing(std::vector<uint32_t> &x, uint32_t number);

void installConfig(superblock_t &superblock, string name);


int main(int argc, char **argv) {
	argc -= (argc > 0);
	argv += (argc > 0);
	option::Stats stats(usage, argc, argv);
	option::Option options[stats.options_max], buffer[stats.buffer_max];
	option::Parser parse(usage, argc, argv, options, buffer);

	if (parse.error())
		return 1;

	if (options[HELP] || argc == 0) {
		option::printUsage(cerr, usage);
		return 0;
	}

	for (option::Option *opt = options[UNKNOWN]; opt; opt = opt->next())
		cerr << "Unknown option: " << opt->name << "\n";
	if (options[UNKNOWN] != NULL)
		return -1;

#ifdef DEBUG
	for (int i = 0; i < parse.nonOptionsCount(); ++i)
		std::cout << "Non-option #" << i << ": " << parse.nonOption(i) << "\n";
#endif

	chdir(HOME_DIR);

	superblock_t superblock;
	memset(&superblock, 0, sizeof(superblock_t));
	superblock.pools.clear();

	string confName;

//	if option new.
	if (options[NEW] != NULL) {
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

	if (options[ICACHE] != NULL) {
		superblock.iCacheSz = stoi(options[ICACHE].arg);
	}

	if (options[DCACHE] != NULL) {
		superblock.dCacheSz = stoi(options[DCACHE].arg);
	}

	if (options[BCACHE] != NULL) {
		superblock.bCacheSz = stoi(options[BCACHE].arg);
	}

	if (options[BSIZE] != NULL) {
		superblock.blockSz = stoi(options[BSIZE].arg);
	}

	if (options[REDUNDANCY] != NULL) {
		superblock.redundancy = stoi(options[REDUNDANCY].arg);
	}

	if (options[MIGRATION] != NULL && options[POOL] == NULL) {
		string migration = options[MIGRATION].arg;

		if ("time" == migration) {
			superblock.migration = Rule::TIME_MIGRATION;
		} else if ("user" == migration) {
			superblock.migration = Rule::RIGHT_MIGRATION;
		} else {
			cerr << "unknow migration";
			return -1;
		}
	} else if (options[MIGRATION] != NULL && options[POOL] != NULL) {
	}

	if (options[ADD] != NULL && options[POOL] == NULL) {
//		ADD one pool
#ifdef DEBUG
		cout << "add a pool" << endl;
#endif
		pool_t pool;
		pool.migration = Rule::TIME_MIGRATION;
		pool.volumes.clear();

		if (options[CONFIG] != NULL) {
			string arg = options[CONFIG].arg;
			Document tmpDoc;
#ifdef DEBUG
			cout << "pool arg: " << arg << endl;
#endif

			if (arg.substr(arg.length() - 5) == ".json") {
//				TODO Parse json file
			} else
				tmpDoc.Parse(arg.c_str());

			if (Rule::rulesAreValid(superblock.migration, tmpDoc) < 0) {
				cerr << "Invalid config!" << endl;
				return -1;
			}

			if (tmpDoc.HasMember(Rule::MIGRATION))
				pool.migration = tmpDoc[Rule::MIGRATION].GetInt();

			pool.rule = Rule::buildRule(superblock.migration, tmpDoc);
		} else {
			cerr << "config needed!" << endl;
			return -1;
		}

		int newPoolId = addPool(superblock, pool);
		if (newPoolId > 0)
			cout << "new pool:" << newPoolId << endl;

	} else if (options[ADD] != NULL && options[POOL] != NULL) {
//		ADD one volume.
		if (superblock.pools.find(stoul(options[POOL].arg)) == superblock.pools.end()) {
			cerr << "pool '" << options[POOL].arg << "' not found" << endl;
			return -1;
		}

		uint32_t poolId = (uint32_t) stoul(options[POOL].arg);
		volume_t volume;
		pool_t *pool = &superblock.pools[poolId];

		if (options[CONFIG] != NULL) {
			string arg = options[CONFIG].arg;
			Document tmpDoc;
#ifdef DEBUG
			cout << "pool arg: " << arg << endl;
#endif

			if (arg.substr(arg.length() - 5) == ".json") {
//				TODO Parse json file
			} else
				tmpDoc.Parse(arg.c_str());

			if (Rule::rulesAreValid(pool->migration, tmpDoc) < 0) {
				cerr << "Invalid config!" << endl;
				return -1;
			}

			if (!tmpDoc.HasMember(pluginSystem::Plugin::TYPE)) {
				cerr << "config need a plugin name eg: {\"plName\":\"block\"}" << endl;
				return -1;
			}
			volume.pluginName = tmpDoc[pluginSystem::Plugin::TYPE].GetString();

			if (tmpDoc.HasMember(pluginSystem::Plugin::PARAMS)) {
				for (auto &&param: tmpDoc[pluginSystem::Plugin::PARAMS].GetObject()) {
					volume.params.insert(make_pair(param.name.GetString(), param.value.GetString()));
				}
			}

			volume.rule = Rule::buildRule(pool->migration, tmpDoc);
		} else {
			cerr << "config needed!" << endl;
			return -1;
		}

		int newVolumeId = addVolume(*pool, volume);
		if (newVolumeId > 0)
			cout << "new volume:" << newVolumeId << endl;
	}

	writeConfig(superblock, confName);

	if (options[INSTALL] != NULL) {
		installConfig(superblock, confName);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////Definitions//////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool configExist(string name) {
	return Fs::fileExists(CONF_DIR, name + ".json");
}

void loadConfig(superblock_t &sb, string name) {
	string filename = string(CONF_DIR) + name + ".json";
	ifstream file(filename);
	if (!file.is_open()) {
		return;
	}

	IStreamWrapper wrapper(file);
	Document confFile;

	confFile.ParseStream(wrapper);

	Mtfs::jsonToStruct(confFile, sb);
}

bool writeConfig(superblock_t &superblock, string confName) {
	Document d;
	Document::AllocatorType &allocator = d.GetAllocator();
	d.SetObject();

	Value v;

	Mtfs::structToJson(superblock, d);

	StringBuffer sb;
	PrettyWriter<StringBuffer> pw(sb);
	d.Accept(pw);

	string filename = string(CONF_DIR) + confName + ".json";
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
	if (ids.size() == 0)
		volumeId = 1;
	else
		volumeId = findMissing(ids, 0);

	if (pool.volumes.find(volumeId) != pool.volumes.end())
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
	if (ids.size() == 0)
		poolId = 1;
	else
		poolId = findMissing(ids, 0);

	if (sb.pools.find(poolId) != sb.pools.end())
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
	else {
		std::vector<int> diffs;
		std::adjacent_difference(pos, x.end(), std::back_inserter(diffs));
		auto pos2 = std::find_if(diffs.begin() + 1, diffs.end(), [](int x) { return x > 1; });
		return *(pos + (pos2 - diffs.begin() - 1)) + 1;
	}
}

void installConfig(superblock_t &superblock, string name) {
	string filename = "superblock.json";
	boost::filesystem::create_directory(INSTALL_DIR + name);
	string src = string(CONF_DIR) + name + ".json";
	string dst = string(INSTALL_DIR) + name + "/" + filename;
	boost::filesystem::copy_file(src, dst, boost::filesystem::copy_option::overwrite_if_exists);
	string rootFilename = string(INSTALL_DIR) + name + "/root.json";

	inode_t rootInode;
	Mtfs::createRootInode(rootInode);

	Document rootJSON(kObjectType);
	rootInode.toJson(rootJSON);

//	Attach all plugins for write superblock and rootInode
//	Only 3 firsts for rootInode
	pluginSystem::PluginManager *manager = pluginSystem::PluginManager::getInstance();
	int i = 0;
	for (auto &&pool: superblock.pools) {
		for (auto &&volume: pool.second.volumes) {
			volume.second.params["home"] = PLUGIN_HOME;
			volume.second.params["blockSize"] = to_string(superblock.blockSz);

			pluginSystem::Plugin *plugin;

			plugin = manager->getPlugin(volume.second.pluginName);
			plugin->attach(volume.second.params);

			if (i < 3)
				plugin->writeInode(0, rootInode);
			plugin->writeSuperblock(superblock);

			i++;
		}
	}

	StringBuffer sb;
	PrettyWriter<StringBuffer> pw(sb);
	rootJSON.Accept(pw);

	ofstream configFile(rootFilename);
	configFile << sb.GetString() << endl;
	configFile.close();
}

