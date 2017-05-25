/**
 * @author David Wittwer
 * @date 30.04.17.
**/
#define FUSE_USE_VERSION 30
#define HOME_DIR "/home/david/Cours/4eme/Travail_bachelor/Home/"
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

using namespace std;

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

void loadConfig(rapidjson::Document &d, string name);

bool writeConfig(rapidjson::Document &d, string confName);

int addPool(rapidjson::Document &d, rapidjson::Value &pool);

int findMissing(std::vector<int> &x, int number);

int main(int argc, char **argv) {
	argc -= (argc > 0);
	argv += (argc > 0);
	option::Stats stats(usage, argc, argv);
	option::Option options[stats.options_max], buffer[stats.buffer_max];
	option::Parser parse(usage, argc, argv, options, buffer);

	if (parse.error())
		return 1;

	if (options[HELP] || argc == 0) {
		option::printUsage(cout, usage);
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

	rapidjson::Document d;
	d.SetObject();
	rapidjson::Document::AllocatorType &allocator = d.GetAllocator();
	rapidjson::Value v;
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

		v.SetInt(4096);
		d.AddMember(rapidjson::StringRef(mtfs::Mtfs::INODE_CACHE), v, allocator);
		v.SetInt(4096);
		d.AddMember(rapidjson::StringRef(mtfs::Mtfs::DIR_CACHE), v, allocator);
		v.SetInt(4096);
		d.AddMember(rapidjson::StringRef(mtfs::Mtfs::BLOCK_CACHE), v, allocator);
		v.SetInt(4096);
		d.AddMember(rapidjson::StringRef(mtfs::Mtfs::BLOCK_SIZE), v, allocator);
		v.SetInt(1);
		d.AddMember(rapidjson::StringRef(mtfs::Mtfs::REDUNDANCY), v, allocator);
		v.SetInt(mtfs::Rule::TIME_MIGRATION);
		d.AddMember(rapidjson::StringRef(mtfs::Rule::MIGRATION), v, allocator);
	} else {
		confName = parse.nonOption(0);
		if (!configExist(confName)) {
			cerr << "Config not exist" << endl;
			return -1;
		}

#ifdef DEBUG
		cout << "Load config " << confName << endl;
#endif

		loadConfig(d, confName);
	}

	if (options[ICACHE] != NULL)
		d[mtfs::Mtfs::INODE_CACHE].SetInt(stoi(options[ICACHE].arg));

	if (options[DCACHE] != NULL)
		d[mtfs::Mtfs::DIR_CACHE].SetInt(stoi(options[DCACHE].arg));

	if (options[BCACHE] != NULL)
		d[mtfs::Mtfs::BLOCK_CACHE].SetInt(stoi(options[BCACHE].arg));

	if (options[BSIZE] != NULL)
		d[mtfs::Mtfs::BLOCK_SIZE].SetInt(stoi(options[BSIZE].arg));

	if (options[REDUNDANCY] != NULL)
		d[mtfs::Mtfs::REDUNDANCY].SetInt(stoi(options[REDUNDANCY].arg));

	if (options[MIGRATION] != NULL && options[POOL] == NULL) {
		string migration = options[MIGRATION].arg;

		if ("time" == migration) {
			d[mtfs::Rule::MIGRATION].SetInt(mtfs::Rule::TIME_MIGRATION);
		} else if ("user" == migration) {
			d[mtfs::Rule::MIGRATION].SetInt(mtfs::Rule::RIGHT_MIGRATION);
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
		rapidjson::Value pool(rapidjson::kObjectType);

		if (options[CONFIG] != NULL) {
			string arg = options[CONFIG].arg;
			rapidjson::Document tmpDoc;
#ifdef DEBUG
			cout << "arg: " << arg << endl;
#endif

			if (arg.substr(arg.length() - 5) == ".json") {
#ifdef DEBUG
				cout << "json file" << endl;
#endif
			} else
				tmpDoc.Parse(arg.c_str());

			if (mtfs::Rule::rulesAreValid(d[mtfs::Rule::MIGRATION].GetInt(), tmpDoc) < 0) {
				cerr << "Invalid config!" << endl;
				return -1;
			}

			mtfs::Rule::copyConfig(d[mtfs::Rule::MIGRATION].GetInt(), tmpDoc, pool, allocator);
		} else {

		}

		v.SetInt(mtfs::Rule::TIME_MIGRATION);
		pool.AddMember(rapidjson::StringRef(mtfs::Rule::MIGRATION), v, allocator);

		int newPoolId = addPool(d, pool);
		if (newPoolId >= 0)
			cout << "new pool:" << newPoolId << endl;

	} else if (options[ADD] != NULL && options[POOL] != NULL) {
//		ADD one volume.
		if (!d[mtfs::Pool::POOLS].HasMember(options[POOL].arg)) {
			cerr << "pool '" << options[POOL].arg << "' not found" << endl;
			return -1;
		}
		rapidjson::Value volume(rapidjson::kObjectType);
		rapidjson::Value pool(d[mtfs::Pool::POOLS][options[POOL].arg], allocator);
		if (options[CONFIG] != NULL) {
			string arg = options[CONFIG].arg;
			rapidjson::Document tmpDoc;
#ifdef DEBUG
			cout << "vol arg: " << arg << endl;
#endif

			if (arg.substr(arg.length() - 5) == ".json") {
#ifdef DEBUG
				cout << "json file" << endl;
#endif
			} else
				tmpDoc.Parse(arg.c_str());

			int migration = pool[mtfs::Rule::MIGRATION].GetInt();

			if (mtfs::Rule::rulesAreValid(migration, tmpDoc) != mtfs::Rule::VALID_RULES) {
				cerr << "Invalid config!" << endl;
				return -1;
			}

			volume = tmpDoc.GetObject();
//			mtfs::Rule::copyConfig(migration, tmpDoc, volume, allocator);
		} else {

		}


		int volumeId;
		rapidjson::Value volumes(rapidjson::kObjectType);
		if (pool.HasMember(mtfs::Volume::VOLUMES)) {
			volumes = pool[mtfs::Volume::VOLUMES].GetObject();
			vector<int> idVector;
			for (auto &m: volumes.GetObject()) {
				idVector.push_back(stoi(m.name.GetString()));
			}
			volumeId = findMissing(idVector, 0);
		} else {
			pool.AddMember(rapidjson::StringRef(mtfs::Volume::VOLUMES), volumes, allocator);
			volumes.SetObject();
			volumeId = 1;
		}

		volumes.AddMember(rapidjson::StringRef(to_string(volumeId).c_str()), volume, allocator);
		pool[mtfs::Volume::VOLUMES] = volumes;
		d[mtfs::Pool::POOLS][options[POOL].arg] = pool;

		cout << "new volume:" << volumeId << endl;
	}


#ifndef DEBUG
	if (mtfs::Mtfs::validate(d))
#endif
	writeConfig(d, confName);

	if (options[INSTALL] != NULL) {
		string filename = confName + ".json";
		boost::filesystem::create_directory(INSTALL_DIR + confName);
		string src = string(CONF_DIR) + filename;
		string dst = string(INSTALL_DIR) + confName + "/" + filename;
		boost::filesystem::copy_file(src, dst, boost::filesystem::copy_option::overwrite_if_exists);
		string rootFilename = string(INSTALL_DIR) + confName + "/root.json";

		rapidjson::Document root(rapidjson::kObjectType);
		mtfs::Mtfs::createRootInode(root);

		rapidjson::StringBuffer sb;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
		root.Accept(pw);

		ofstream configFile(rootFilename);
		configFile << sb.GetString() << endl;
		configFile.close();
	}

	return 0;
}

bool configExist(string name) {
	return Fs::fileExists(CONF_DIR, name + ".json");
}

void loadConfig(rapidjson::Document &d, string name) {
	string filename = string(CONF_DIR) + name + ".json";
	ifstream file(filename);
	if (!file.is_open()) {
		return;
	}

	rapidjson::IStreamWrapper wrapper(file);

	d.ParseStream(wrapper);
}

bool writeConfig(rapidjson::Document &d, string confName) {
	rapidjson::StringBuffer sb;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
	d.Accept(pw);

	string filename = string(CONF_DIR) + confName + ".json";
	ofstream configFile(filename);
	configFile << sb.GetString() << endl;
	configFile.close();

	return true;
}

/**
 * @brief add a pool in system
 *
 * @param d 	The json config
 * @param pool 	The json pool to add
 *
 * @return
 */
int addPool(rapidjson::Document &d, rapidjson::Value &pool) {
	rapidjson::Value pools(rapidjson::kObjectType);
	int poolId;
	if (d.HasMember(mtfs::Pool::POOLS)) {
		pools = d[mtfs::Pool::POOLS].GetObject();
		vector<int> idVector;
		for (auto &m: pools.GetObject()) {
			idVector.push_back(stoi(m.name.GetString()));
		}
		poolId = findMissing(idVector, 0);
	} else {
		d.AddMember(rapidjson::StringRef(mtfs::Pool::POOLS), pools, d.GetAllocator());
		pools.SetObject();
		poolId = 1;
	}

	pools.AddMember(rapidjson::StringRef(to_string(poolId).c_str()), pool, d.GetAllocator());
	d[mtfs::Pool::POOLS] = pools;


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
int findMissing(std::vector<int> &x, int number) {
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