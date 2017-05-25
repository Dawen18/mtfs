#include <mtfs/Cache.h>
#include <mtfs/Mtfs.h>
#include <pluginSystem/PluginManager.h>

namespace mtfs {
	using namespace std;
	using namespace rapidjson;
	Mtfs *Mtfs::instance = 0;
	thread *Mtfs::thr = 0;
	std::mutex Mtfs::mutex;
	bool Mtfs::keepRunning;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////											STATICS															////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Mtfs::validate(const Value &system) {
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
			if (Rule::rulesAreValid(migration, m.value) != Rule::VALID_RULES)
				throw invalid_argument(string("Rules invalid for pool '") + m.name.GetString() + "'");

			if (!Pool::validate(m.value))
				throw invalid_argument(string("Pool '") + m.name.GetString() + "' invalid!");
		}

		return true;
	}

	bool Mtfs::createRootInode(Document &d) {
		inode_t inode;
		memset(&inode, 0, sizeof(inode));
		inode.linkCount = 1;
		inode.uid = 0;
		inode.gid = 0;
		inode.accesRight = 0777;

		d.SetObject();
		Document::AllocatorType &alloc = d.GetAllocator();

		Value v;

		v.SetUint(inode.accesRight);
		d.AddMember(StringRef("accessRight"), v, alloc);

		v.SetUint(inode.uid);
		d.AddMember(StringRef("uid"), v, alloc);

		v.SetUint(inode.gid);
		d.AddMember(StringRef("gid"), v, alloc);

		v.SetUint64(inode.size);
		d.AddMember(StringRef("size"), v, alloc);

		v.SetUint(inode.linkCount);
		d.AddMember(StringRef("linkCount"), v, alloc);

		v.SetUint64(inode.access);
		d.AddMember(StringRef("access"), v, alloc);

		Value a(kArrayType);

		for (auto b = inode.referenceId.begin(); b != inode.referenceId.end(); b++) {
			Value o(kObjectType);

			mtfs::ident_t ident = *b;
			v.SetUint(ident.poolId);
			o.AddMember(StringRef("poolId"), v, alloc);
			v.SetUint(ident.volumeId);
			o.AddMember(StringRef("volumeId"), v, alloc);
			v.SetUint64(ident.id);
			o.AddMember(StringRef("id"), v, alloc);

			a.PushBack(o, alloc);
		}

		d.AddMember(StringRef("referenceId"), a, alloc);

		Value ba(kArrayType);
		for (auto b = inode.dataBlocks.begin(); b != inode.dataBlocks.end(); b++) {
			vector<ident_t> redondancy = *b;
			Value ra(kArrayType);
			for (auto be = redondancy.begin(); be != redondancy.end(); be++) {
				Value ids(kObjectType);

				ident_t ident = *be;
				v.SetUint(ident.poolId);
				ids.AddMember(StringRef("poolId"), v, alloc);
				v.SetUint(ident.volumeId);
				ids.AddMember(StringRef("volumeId"), v, alloc);
				v.SetUint64(ident.id);
				ids.AddMember(StringRef("id"), v, alloc);

				ra.PushBack(ids, alloc);
			}

			ba.PushBack(ra, alloc);
		}

		d.AddMember(StringRef("dataBlocks"), ba, alloc);

		return true;
	}

	Mtfs *Mtfs::getInstance() {
		if (!instance)
			instance = new Mtfs();

		return instance;
	}

	void Mtfs::start() {
		std::unique_lock<std::mutex> lock(mutex);
		if (keepRunning)
			return;

		keepRunning = true;
		thr = new std::thread(&loop);
	}

	void Mtfs::join() {
		std::unique_lock<std::mutex> lock(mutex);
		keepRunning = false;
		lock.unlock();

		thr->join();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////											MEMBERS															////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Mtfs::build(const Value &system, string homeDir) {
		if (!validate(system))
			return false;

		int mainMigration = mtfs::Rule::NO_MIGRATION;
		if (system[Pool::POOLS].MemberCount() != 1)
			mainMigration = system[mtfs::Rule::MIGRATION].GetInt();

		pluginSystem::PluginManager *pluginManager = pluginSystem::PluginManager::getInstance();

		//	Instatiate classes
		mtfs::PoolManager *poolManager = new mtfs::PoolManager();

		//	iter pools
		for (auto &p: system[mtfs::Pool::POOLS].GetObject()) {
			cout << "poolstr " << p.name.GetString() << endl;
//			int poolId = stoi(p.name.GetString());
//			cout << "poolId: " << poolId << endl;

			mtfs::Pool *pool = new mtfs::Pool();

			int volMigration = mtfs::Rule::NO_MIGRATION;
			if (p.value[mtfs::Volume::VOLUMES].MemberCount() != 1)
				volMigration = p.value[mtfs::Rule::MIGRATION].GetInt();

			//		iter volumes
			for (auto &v: p.value.GetObject()[mtfs::Volume::VOLUMES].GetObject()) {
				cout << "volstr " << v.name.GetString() << endl;

//				int volumeId = stoi(v.name.GetString());
#ifdef DEBUG
//				cout << "\tvolumeId: " << volumeId << " type: " << v.value[pluginSystem::Plugin::TYPE].GetString()
//					 << endl;
#endif

				pluginSystem::Plugin *plugin = pluginManager->getPlugin(
						v.value[pluginSystem::Plugin::TYPE].GetString());
				map<string, string> params;
				params["home"] = homeDir + "/Plugins";
				params["blockSize"] = to_string(system[Mtfs::BLOCK_SIZE].GetInt());

				//			Construct params
				vector<string> neededParams = plugin->getInfos();
				neededParams.erase(neededParams.begin());
				for (auto &param: neededParams) {
					//				cout << param << endl;
					params[param] = v.value[param.c_str()].GetString();
					cout << param << " ";
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

	void Mtfs::processSynchronous() {
		string message = synchronousQueue->front();
		synchronousQueue->pop();

		cout << "mtfs receive <" << message << ">" << endl;
	}

	void Mtfs::setSynchronousQueue(ThreadQueue<string> *synchronousQueue) {
		Mtfs::synchronousQueue = synchronousQueue;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////											PRIVATE															////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Mtfs::Mtfs() {
		keepRunning = false;
	}

	void Mtfs::loop() {

		Mtfs *mtfs = getInstance();

		std::unique_lock<std::mutex> lock(mutex);
		while (keepRunning) {
			mutex.unlock();
			mtfs->processSynchronous();
			mutex.lock();
		}
	}


}  // namespace mtfs
