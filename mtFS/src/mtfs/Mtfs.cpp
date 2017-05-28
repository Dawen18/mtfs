#include <mtfs/Cache.h>
#include <mtfs/Mtfs.h>
#include <pluginSystem/PluginManager.h>
#include <fstream>
#include <rapidjson/istreamwrapper.h>

namespace mtfs {
	using namespace std;
	using namespace rapidjson;
	using namespace boost::threadpool;

	Mtfs *Mtfs::instance = 0;
	pool *Mtfs::threadPool = 0;
	string Mtfs::systemName;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////											STATICS															////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Mtfs *Mtfs::getInstance() {
		if (!instance)
			instance = new Mtfs();

		return instance;
	}

	bool Mtfs::validate(const Value &system) {
		if (!system.IsObject())
			throw invalid_argument("Not a object!");

		if (!system.HasMember(INODE_CACHE))
			throw invalid_argument("Inode cache missing!");

		if (!system.HasMember(DIR_CACHE))
			throw invalid_argument("Directory cache missing!");

		if (!system.HasMember(BLOCK_CACHE))
			throw invalid_argument("Block cache missing!");

		if (!system.HasMember(BLOCK_SIZE_ST))
			throw invalid_argument("Block size missing!");

		if (!system.HasMember(REDUNDANCY))
			throw invalid_argument("Redundancy missing!");

		if (!system.HasMember(Pool::POOLS))
			throw invalid_argument("Pools missing!");
		if (!system[Pool::POOLS].IsObject())
			throw invalid_argument("Pool is not a object!");

		int migration = -1;
		if (system[Pool::POOLS].MemberCount() <= 0)
			throw invalid_argument("Number of pool invalid!");
		else if (system[Pool::POOLS].MemberCount() != 1) {
			if (!system.HasMember(Rule::MIGRATION))
				throw invalid_argument("Migration missing!");

			migration = system[Rule::MIGRATION].GetInt();
		}

		for (auto &m: system[Pool::POOLS].GetObject()) {
			if (Rule::rulesAreValid(migration, m.value) != Rule::VALID_RULES)
				throw invalid_argument(string("Rules invalid for pool '") + m.name.GetString() + "'");

			if (!Pool::validate(m.value))
				throw invalid_argument(string("Pool '") + m.name.GetString() + "' invalid!");
		}

		return true;
	}

	bool Mtfs::createRootInode(inode_t &inode) {
		inode.accesRight = 0777;
		inode.uid = 0;
		inode.gid = 0;
		inode.size = 1024;
		inode.linkCount = 2;
		inode.access = (uint64_t) time(NULL);
		inode.referenceId.clear();
		inode.dataBlocks.clear();

		return true;
	}

	bool Mtfs::start(const rapidjson::Value &system, std::string homeDir, string sysName) {
		systemName = sysName;

		if (!validate(system) || !getInstance()->build(system, homeDir))
			return false;

		unsigned nbThread = (unsigned int) (thread::hardware_concurrency() * 1.25);
		threadPool = new pool(nbThread);

		return true;
	}

	void Mtfs::stop() {
		delete threadPool;

		delete instance;
	}

	void Mtfs::structToJson(const superblock_t &sb, rapidjson::Document &d) {
		rapidjson::Document::AllocatorType &allocator = d.GetAllocator();

		d.SetObject();

		rapidjson::Value v;

		v.SetInt(sb.iCacheSz);
		d.AddMember(rapidjson::StringRef(Mtfs::INODE_CACHE), v, allocator);

		v.SetInt(sb.dCacheSz);
		d.AddMember(rapidjson::StringRef(Mtfs::DIR_CACHE), v, allocator);

		v.SetInt(sb.bCacheSz);
		d.AddMember(rapidjson::StringRef(Mtfs::BLOCK_CACHE), v, allocator);

		v.SetInt(sb.blockSz);
		d.AddMember(rapidjson::StringRef(Mtfs::BLOCK_SIZE_ST), v, allocator);

		v.SetInt(sb.redundancy);
		d.AddMember(rapidjson::StringRef(Mtfs::REDUNDANCY), v, allocator);

		v.SetInt(sb.migration);
		d.AddMember(rapidjson::StringRef(Rule::MIGRATION), v, allocator);

		rapidjson::Value pools(rapidjson::kObjectType);
		for (auto &&item : sb.pools) {
			rapidjson::Value pool(rapidjson::kObjectType);

			Pool::structToJson(item.second, pool, allocator);

			string id = to_string(item.first);
			Value index(id.c_str(), (SizeType) id.size(), allocator);
			pools.AddMember(index, pool, allocator);
		}
		d.AddMember(rapidjson::StringRef(Pool::POOLS), pools, allocator);
	}

	void Mtfs::jsonToStruct(rapidjson::Document &d, superblock_t &sb) {
		assert(d.HasMember(INODE_CACHE));
		sb.iCacheSz = d[INODE_CACHE].GetInt();

		assert(d.HasMember(DIR_CACHE));
		sb.dCacheSz = d[DIR_CACHE].GetInt();

		assert(d.HasMember(BLOCK_CACHE));
		sb.bCacheSz = d[BLOCK_CACHE].GetInt();

		assert(d.HasMember(BLOCK_SIZE_ST));
		sb.blockSz = d[BLOCK_SIZE_ST].GetInt();

		assert(d.HasMember(REDUNDANCY));
		sb.redundancy = d[REDUNDANCY].GetInt();

		assert(d.HasMember(Rule::MIGRATION));
		sb.migration = d[Rule::MIGRATION].GetInt();

		assert(d.HasMember(Pool::POOLS));
		for (auto &&item :d[Pool::POOLS].GetObject()) {
			string sId = item.name.GetString();
			uint32_t id = (uint32_t) stoul(sId);
			pool_t pool;
			memset(&pool, 0, sizeof(pool_t));
			pool.volumes.clear();

			pool.rule = Rule::buildRule(sb.migration, item.value);

			Pool::jsonToStruct(item.value, pool);

			sb.pools.insert(make_pair(id, pool));
		}
	}

	void Mtfs::getAttr(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		(void) fi;

		if (ino == FUSE_ROOT_ID) {
//			cout << "root asked" << endl;

			struct stat rootStats;
			memset(&rootStats, 0, sizeof(struct stat));

			if (instance->rootStat(rootStats) != 0)
				fuse_reply_err(req, errno);

			fuse_reply_attr(req, &rootStats, 1.0);
		} else
			threadPool->schedule(bind(&Mtfs::stat, instance, req, ino));
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////											MEMBERS															////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////											PRIVATE															////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Mtfs::Mtfs() {
	}

	bool Mtfs::build(const Value &system, string homeDir) {

		int mainMigration = mtfs::Rule::NO_MIGRATION;
		if (system[Pool::POOLS].MemberCount() != 1)
			mainMigration = system[mtfs::Rule::MIGRATION].GetInt();

		pluginSystem::PluginManager *pluginManager = pluginSystem::PluginManager::getInstance();

		//	Instatiate classes
		mtfs::PoolManager *poolManager = new mtfs::PoolManager();

		//	iter pools
		for (auto &p: system[mtfs::Pool::POOLS].GetObject()) {
#ifdef DEBUG
//			cout << "poolstr " << p.name.GetString() << endl;
//			int poolId = stoi(p.name.GetString());
//			cout << "poolId: " << poolId << endl;
#endif

			mtfs::Pool *pool = new mtfs::Pool();

			int volMigration = mtfs::Rule::NO_MIGRATION;
			if (p.value[mtfs::Volume::VOLUMES].MemberCount() != 1)
				volMigration = p.value[mtfs::Rule::MIGRATION].GetInt();

			//		iter volumes
			for (auto &v: p.value.GetObject()[mtfs::Volume::VOLUMES].GetObject()) {

//				int volumeId = stoi(v.name.GetString());
#ifdef DEBUG
//				cout << "volstr " << v.name.GetString() << endl;
//				cout << "\tvolumeId: " << volumeId << " type: " << v.value[pluginSystem::Plugin::TYPE].GetString()
//					 << endl;
#endif

				pluginSystem::Plugin *plugin = pluginManager->getPlugin(
						v.value[pluginSystem::Plugin::TYPE].GetString());
				map<string, string> params;
				params["home"] = homeDir + "/Plugins";
				params["blockSize"] = to_string(system[Mtfs::BLOCK_SIZE_ST].GetInt());

				//			Construct params
				vector<string> neededParams = plugin->getInfos();
				neededParams.erase(neededParams.begin());
				for (auto &param: neededParams) {
//					cout << param << endl;
					params[param] = v.value[param.c_str()].GetString();
//					cout << param << " ";
				}

				if (!plugin->attach(params)) {
					cerr << "Failed to attach plugin " << plugin->getInfos()[0] << endl;
					delete (plugin);

					return -1;
				}

				mtfs::Volume *volume = new mtfs::Volume(plugin);

				pool->addVolume((uint32_t) stoul(v.name.GetString()), volume,
								mtfs::Rule::buildRule(volMigration, v.value));
			}
			poolManager->addPool((uint32_t) stoul(p.name.GetString()), pool,
								 mtfs::Rule::buildRule(mainMigration, p.value));
		}

		this->inodes = poolManager;
		this->dirEntries = poolManager;
		this->blocks = poolManager;

		return true;
	}

	void Mtfs::stat(fuse_req_t req, fuse_ino_t ino) {
		(void) ino;

		fuse_reply_err(req, ENOSYS);
	}

	int Mtfs::rootStat(struct stat &st) {
		string filename = string(SYSTEMS_DIR) + "/" + systemName + "/root.json";
		ifstream file(filename);
		if (!file.is_open())
			return -1;

		IStreamWrapper wrapper(file);

		Document d(kObjectType);
		d.ParseStream(wrapper);


		st.st_dev = 0;
		st.st_ino = 0;

		assert(d.HasMember(IN_MODE));
		st.st_ino = d[IN_MODE].GetUint();

		assert(d.HasMember(IN_LINKS));
		st.st_nlink = d[IN_LINKS].GetUint();

		assert(d.HasMember(IN_UID));
		st.st_uid = d[IN_UID].GetUint();

		assert(d.HasMember(IN_GID));
		st.st_gid = d[IN_GID].GetUint();

		st.st_size = 1024;

		assert(d.HasMember(IN_ACCESS));
		time_t time = d[IN_ACCESS].GetUint64();
		st.st_atim.tv_sec = time;
		st.st_ctim.tv_sec = time;
		st.st_mtim.tv_sec = time;


		return 0;
	}

}  // namespace mtfs
