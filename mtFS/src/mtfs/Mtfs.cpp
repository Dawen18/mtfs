#include <mtfs/Cache.h>
#include <mtfs/Mtfs.h>
#include <pluginSystem/PluginManager.h>

namespace mtfs {
	using namespace std;
	Mtfs *Mtfs::instance = 0;
	thread *Mtfs::thr = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////											STATICS															////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Mtfs::validate(const rapidjson::Value &system) {
		if (!system.IsObject())
			throw invalid_argument("Not a object!");

		if (!system.HasMember(INODE_CACHE))
			throw invalid_argument("Inode cache missing!");

		if (!system.HasMember(DIR_CACHE))
			throw invalid_argument("Directory cache missing!");

		if (!system.HasMember(BLOCK_CACHE))
			throw invalid_argument("Block cache missing!");

		if (!system.HasMember(BLOCK_SIZE))
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
			if (!Rule::rulesAreValid(migration, m.value))
				throw invalid_argument(string("Rules invalid for pool '") + m.name.GetString() + "'");

			if (!Pool::validate(m.value))
				throw invalid_argument(string("Pool '") + m.name.GetString() + "' invalid!");
		}

		return true;
	}

	Mtfs *Mtfs::getInstance() {
		if (!instance)
			instance = new Mtfs();

		return instance;
	}

	void Mtfs::start() {
		thr = new std::thread(&loop);
	}

	void Mtfs::join() {
		thr->join();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////											MEMBERS															////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Mtfs::build(const rapidjson::Value &system, string homeDir) {
		if (!validate(system))
			return false;

		int mainMigration = mtfs::Rule::NO_MIGRATION;
		if (system[mtfs::Pool::POOLS].MemberCount() != 1)
			mainMigration = system[mtfs::Rule::MIGRATION].GetInt();

		pluginSystem::PluginManager *pluginManager = pluginSystem::PluginManager::getInstance();

		//	Instatiate classes
		mtfs::PoolManager *poolManager = new mtfs::PoolManager();

		//	iter pools
		for (auto &p: system[mtfs::Pool::POOLS].GetObject()) {
			int poolId = stoi(p.name.GetString());
			cout << "poolId: " << poolId << endl;

			mtfs::Pool *pool = new mtfs::Pool();

			int volMigration = mtfs::Rule::NO_MIGRATION;
			if (p.value[mtfs::Volume::VOLUMES].MemberCount() != 1)
				volMigration = p.value[mtfs::Rule::MIGRATION].GetInt();

			//		iter volumes
			for (auto &v: p.value.GetObject()[mtfs::Volume::VOLUMES].GetObject()) {

				int volumeId = stoi(v.name.GetString());
#ifdef DEBUG
				cout << "\tvolumeId: " << volumeId << " type: " << v.value[pluginSystem::Plugin::TYPE].GetString()
					 << endl;
#endif

				pluginSystem::Plugin *plugin = pluginManager->getPlugin(
						v.value[pluginSystem::Plugin::TYPE].GetString());
				map<string, string> params;
				params["home"] = homeDir + "/Plugins";

				//			Construct params
				vector<string> neededParams = plugin->getInfos();
				neededParams.erase(neededParams.begin());
				for (auto &param: neededParams) {
					//				cout << param << endl;
					params[param] = v.value[param.c_str()].GetString();
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

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////											PRIVATE															////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Mtfs::Mtfs() {
//		PoolManager *poolManager = new PoolManager();
//		inodes = poolManager;
//		dirEntry = poolManager;
//		blocs = poolManager;
	}

	void Mtfs::loop() {

		Mtfs *mtfs = getInstance();
		int loc = 0;
		std::cout << "thread\n";
		for (int i = 0; i < 10; ++i) {
			std::this_thread::sleep_for(std::chrono::milliseconds(i));
			loc++;
			std::cout << loc << "\n";
		}

	}
}  // namespace mtfs
