#include <mtfs/Mtfs.h>
#include <mtfs/Cache.h>
#include <pluginSystem/PluginManager.h>
#include <fstream>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <boost/filesystem.hpp>
#include <grp.h>
#include <pwd.h>
#include <mutex>
#include <condition_variable>

#define HOME "/home/david/Cours/4eme/Travail_bachelor/Home/"
#define SYSTEMS_DIR "/home/david/Cours/4eme/Travail_bachelor/Home/Systems/"
#define PLUGIN_HOME "/home/david/Cours/4eme/Travail_bachelor/Home/Plugins/"

#define GROUPS_TO_SEARCH 30
//TODO compute this value in function of entry size
#define ENTRY_PER_BLOCK 20

namespace mtfs {
	using namespace std;
	using namespace rapidjson;
	using namespace boost::threadpool;

	struct mtfs_req_st {
		Semaphore *sem;
		int status;

		mtfs_req_st(int st) : status(st) {}

		~mtfs_req_st() {
			delete this->sem;
		}
	};

	struct internalInode_st {
		vector<ident_t> idents;
		inode_t inode;
	};

	struct dl_st {
		const inode_t ino;
		mutex &mu;
		bool &continueDl;
		bool &ended;
		const Semaphore &sem;
		union {
			queue<dirBlock_t *> &dir;
			queue<uint8_t *> &blk;
		} fifo;
		int status;
	};

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
		inode.accesRight = S_IFDIR | 0775;
		inode.uid = 0;
		inode.gid = 1001;
		inode.size = 0;
		inode.linkCount = 2;
		inode.atime = (uint64_t) time(NULL);
		inode.referenceId.clear();
		inode.dataBlocks.clear();

		return true;
	}

	bool Mtfs::start(Document &system, std::string homeDir, string sysName) {
		(void) homeDir;
		systemName = sysName;

		superblock_t superblock;
		jsonToStruct(system, superblock);

		if (!getInstance()->build(superblock))
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
		for (auto &&item: sb.pools) {
			rapidjson::Value pool(rapidjson::kObjectType);

			Pool::structToJson(item.second, pool, allocator);

			string id = to_string(item.first);
			Value index(id.c_str(), (SizeType) id.size(), allocator);
			pools.AddMember(index, pool, allocator);
		}
		d.AddMember(rapidjson::StringRef(Pool::POOLS), pools, allocator);

		v.SetArray();
		for (auto &&id: sb.rootInodes) {
			Value ident(kObjectType);
			id.toJson(ident, allocator);
			v.PushBack(ident, allocator);
		}
		d.AddMember(StringRef(ROOT_INODES), v, allocator);
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

		assert(d.HasMember(ROOT_INODES) && d[ROOT_INODES].IsArray());
		for (auto &&ident: d[ROOT_INODES].GetArray()) {
			ident_t id;
			id.fromJson(ident);
			sb.rootInodes.push_back(id);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////											MEMBERS															////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////											FUSE fcts														////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Mtfs::init(void *userdata, fuse_conn_info *conn) {
		(void) userdata, conn;

		getInstance()->readRootInode();

#ifdef DEBUG
		cerr << "[MTFS}: End init" << endl;
#endif
	}

	void Mtfs::destroy(void *userdata) {
		(void) userdata;
//		uint64_t iptr = (uint64_t) this->inodes;
//		uint64_t dptr = (uint64_t) this->dirBlocks;
//		uint64_t bptr = (uint64_t) this->blocks;

//		if (iptr != dptr && iptr != bptr)
//			delete this->inodes;

//		if (dptr != bptr)
//			delete this->dirBlocks;

		delete this->blocks;
	}

	void Mtfs::getAttr(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		(void) fi;

		chdir(HOME);

		if (ino == FUSE_ROOT_ID) {
			struct stat rootStats;
			memset(&rootStats, 0, sizeof(struct stat));

			if (instance->rootStat(rootStats) != 0) {
				cerr << "[MTFS]: error during root stat" << endl;
				fuse_reply_err(req, errno);
			}

			fuse_reply_attr(req, &rootStats, 1.0);
		} else
			threadPool->schedule(bind(&Mtfs::stat, instance, req, ino));
	}

	void Mtfs::setAttr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int toSet, fuse_file_info *fi) {
		(void) fi;

		internalInode_st *inInode = this->getIntInode(ino);

		if (0 != (FUSE_SET_ATTR_MODE & toSet))
			inInode->inode.accesRight = attr->st_mode;

		if (0 != (FUSE_SET_ATTR_UID & toSet))
			inInode->inode.uid = attr->st_uid;

		if (0 != (FUSE_SET_ATTR_GID & toSet))
			inInode->inode.gid = attr->st_gid;

		if (0 != (FUSE_SET_ATTR_SIZE & toSet))
			inInode->inode.size = (uint64_t) attr->st_size;

		if (0 != (FUSE_SET_ATTR_ATIME & toSet))
			inInode->inode.atime = (uint64_t) attr->st_atim.tv_sec;

		struct stat st;

		buildStat(*inInode, st);
		fuse_reply_attr(req, &st, 1.0);
	}

	void Mtfs::lookup(fuse_req_t req, fuse_ino_t parent, const string name) {
		cout << "parent: " << parent << " name: " << name << endl;

		internalInode_st *parentInode = this->getIntInode(parent);

		pool dlPool((size_t) this->SIMULT_DL);
		Semaphore semaphore;
		queue<dirBlock_t> blkQueue;
		unsigned long sz1 = blkQueue.size();
		mutex queueMutex;
//		dl All blocks
		for (auto &&blks: parentInode->inode.dataBlocks) {
			dlPool.schedule(bind(&Mtfs::dlDirBlocks, this, blks, &blkQueue, &queueMutex, &semaphore));
		}

		vector<ident_t> inodeIds;

		while (1) {
//			wait valid block
			semaphore.wait();
			unique_lock<mutex> lk(queueMutex);
			dirBlock_t block = blkQueue.front();
			blkQueue.pop();
			lk.unlock();

//			id entry is find
			if (block.entries.end() != block.entries.find(name)) {
				dlPool.clear();
				inodeIds = block.entries[name];
//				delete block;
				break;
			}

//			delete block;

//			if queue is empty and pool is empty.
			lk.lock();
			if (blkQueue.empty() && dlPool.empty() && 0 == dlPool.active())
				break;

			lk.unlock();
		}

		inode_t inode = inode_t();
		int ret = 1;

		if (0 == inodeIds.size()) {
			fuse_reply_err(req, ENOENT);
			return;
		}

		do {
			ret = this->inodes->getInode(inodeIds.back(), inode);
			inodeIds.pop_back();
			if (inodeIds.empty())
				break;
		} while (0 != ret);

//		Send reply to fuse
		if (0 != ret)
			fuse_reply_err(req, ret);
		else {
			fuse_entry_param param = fuse_entry_param();
			this->buildParam(inode, param);
			fuse_reply_entry(req, &param);
		}

//		free all datas;
//		dlPool.wait();

//		while (!blkQueue.empty()) {
//			dirBlock_t *db = blkQueue.front();
//			blkQueue.pop();
//			delete db;
//		}
	}

	void Mtfs::mknod(fuse_req_t req, fuse_ino_t parent, const std::string name, mode_t mode, dev_t rdev) {
		(void) rdev;

		int ret;

		internalInode_st *parentInode = nullptr;

		if (parent == FUSE_ROOT_ID) {
			parentInode = this->rootIn;
		} else {
			parentInode = (internalInode_st *) parent;
		}

//		Create and write new inode
		inode_t *inode = this->newInode(mode, fuse_req_ctx(req));
		vector<ident_t> inodeIdents;
		if (0 != (ret = this->insertInode(*inode, inodeIdents))) {
			delete inode;
			fuse_reply_err(req, ret);
			return;
		}

//			Add entry in dir
		if (0 != (ret = this->addEntry(parentInode, name, inodeIdents))) {
			delete inode;
			fuse_reply_err(req, ret);
			return;
		}

//			reply to fuse
		fuse_entry_param param;
		memset(&param, 0, sizeof(fuse_entry_param));

		this->buildParam(*inode, param);

		fuse_reply_entry(req, &param);
	}

	void Mtfs::access(fuse_req_t req, fuse_ino_t ino, int mask) {

		const struct fuse_ctx *context = fuse_req_ctx(req);

		if (FUSE_ROOT_ID == ino) {
			int ret = EACCES;
			inode_t inode = instance->getRootInode();
			if (context->uid == inode.uid) {
				if ((mask << 6 & inode.accesRight) != 0)
					ret = 0;
				else {
					cerr << "[MTFS]: bad user right: " << (inode.accesRight & (mask << 6)) << endl;
				}
			} else {

//			get all user groups;
				size_t bufsize = (size_t) sysconf(_SC_GETPW_R_SIZE_MAX);
				if (bufsize == -1)
					bufsize = 16384;

				char *buf = (char *) malloc(bufsize * sizeof(char));
				if (buf == NULL) {
					fuse_reply_err(req, ENOMEM);
					free(buf);
					return;
				}

				struct passwd pwd, *result;
				int s = getpwuid_r(context->uid, &pwd, buf, bufsize, &result);
				if (NULL == result) {
					int err;
					if (0 == s)
						err = EAGAIN;
					else
						err = s;
					fuse_reply_err(req, err);
					free(buf);
					return;
				}

				int ngroups = GROUPS_TO_SEARCH;
				gid_t *groups = (gid_t *) malloc(ngroups * sizeof(gid_t));
				if (getgrouplist(pwd.pw_name, pwd.pw_gid, groups, &ngroups) == -1) {
					cerr << "get group list error. ngoups " << ngroups << endl;
					free(buf);
					free(groups);
				}

				if (find(groups, groups + ngroups, inode.gid) != groups + ngroups) {
					if ((mask << 3 & inode.accesRight) != 0)
						ret = 0;
				} else {
					if ((mask & inode.accesRight) != 0)
						ret = 0;
				}

				free(buf);
				free(groups);
			}

			fuse_reply_err(req, ret);


		} else {
			fuse_reply_err(req, ENOSYS);
		}
	}

	void Mtfs::opendir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		(void) ino, fi;
		fuse_reply_open(req, fi);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////											PRIVATE															////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Mtfs::Mtfs() : redundancy(1), maxEntryPerBlock(ENTRY_PER_BLOCK) {
		this->inodes = nullptr;
		this->dirBlocks = nullptr;
		this->blocks = nullptr;
		this->rootIn = new internalInode_st();
	}

	inode_t Mtfs::getRootInode() {
		return this->rootIn->inode;
//		return this->rootInode;
	}

	bool Mtfs::build(const superblock_t &superblock) {

		this->redundancy = superblock.redundancy;
		this->blockSize = superblock.blockSz;
		this->rootIn->idents = superblock.rootInodes;

		pluginSystem::PluginManager *manager = pluginSystem::PluginManager::getInstance();

		PoolManager *poolManager = new PoolManager();
		for (auto &&poolSt: superblock.pools) {
			Pool *pool = new Pool();

			for (auto volSt: poolSt.second.volumes) {
				volSt.second.params.insert(make_pair("home", PLUGIN_HOME));
				volSt.second.params.insert(make_pair("blockSize", to_string(this->blockSize)));

				pluginSystem::Plugin *plugin = manager->getPlugin(volSt.second.pluginName);

				if (!plugin->attach(volSt.second.params)) {
					cerr << "Failed attach plugin " << volSt.second.pluginName << endl;
				}

				pool->addVolume(volSt.first, new Volume(plugin), volSt.second.rule);
			}

			poolManager->addPool(poolSt.first, pool, poolSt.second.rule);
		}

		this->inodes = poolManager;
		this->dirBlocks = poolManager;
		this->blocks = poolManager;

		return true;
	}

	void Mtfs::readRootInode() {
		string filename = string(SYSTEMS_DIR) + "/" + systemName + "/root.json";
		ifstream file(filename);
		if (!file.is_open())
			return;

		IStreamWrapper wrapper(file);

		Document d(kObjectType);
		d.ParseStream(wrapper);

		assert(d.HasMember(IN_MODE));
		this->rootIn->inode.accesRight = d[IN_MODE].GetUint();

		assert(d.HasMember(IN_LINKS));
		this->rootIn->inode.linkCount = (uint8_t) d[IN_LINKS].GetUint();

		assert(d.HasMember(IN_UID));
		this->rootIn->inode.uid = d[IN_UID].GetUint();

		assert(d.HasMember(IN_GID));
		this->rootIn->inode.gid = d[IN_GID].GetUint();

		assert(d.HasMember(IN_SIZE));
		this->rootIn->inode.size = d[IN_SIZE].GetUint64();

		assert(d.HasMember(IN_ACCESS));
		this->rootIn->inode.atime = d[IN_ACCESS].GetUint64();

		this->rootIn->inode.referenceId.clear();

		assert(d.HasMember(IN_BLOCKS));
		for (auto &&item: d[IN_BLOCKS].GetArray()) {
			vector<ident_t> blocksRedundancy;
			for (auto &&block: item.GetArray()) {
				ident_t ident;

				assert(block.HasMember(ID_POOL));
				ident.poolId = block[ID_POOL].GetUint();

				assert(block.HasMember(ID_VOLUME));
				ident.volumeId = block[ID_VOLUME].GetUint();

				assert(block.HasMember(ID_ID));
				ident.id = block[ID_ID].GetUint64();

				blocksRedundancy.push_back(ident);
			}
			this->rootIn->inode.dataBlocks.push_back(blocksRedundancy);
		}
	}

	void Mtfs::writeRootInode() {
		Document d;

		this->rootIn->inode.toJson(d);

		StringBuffer sb;
		PrettyWriter<StringBuffer> pw(sb);
		d.Accept(pw);

		string filename = string(SYSTEMS_DIR) + "/" + systemName + "/root.json";
		ofstream rootFile(filename);
		rootFile << sb.GetString() << endl;
		rootFile.close();
	}

	void Mtfs::stat(fuse_req_t req, fuse_ino_t ino) {
		(void) ino;

		fuse_reply_err(req, ENOSYS);
	}

	int Mtfs::rootStat(struct stat &st) {

		st.st_dev = 0;
		st.st_ino = FUSE_ROOT_ID;

		st.st_mode = this->rootIn->inode.accesRight;
		st.st_nlink = this->rootIn->inode.linkCount;
		st.st_uid = this->rootIn->inode.uid;
		st.st_gid = this->rootIn->inode.gid;
		st.st_size = this->rootIn->inode.size;
		time_t time = this->rootIn->inode.atime;
		st.st_atim.tv_sec = time;
		st.st_ctim.tv_sec = time;
		st.st_mtim.tv_sec = time;
		st.st_blksize = this->blockSize;
		st.st_blocks = this->rootIn->inode.dataBlocks.size();

		return 0;
	}

	/**
	 * @brief Add entry in directory.
	 *
	 * This function add a new entry in directory and update parent inode if necessary.
	 *
	 * @param parentInode 	Directory to add entry
	 * @param name 			Name of entry
	 * @param inodeIds 		Inodes od entry
	 *
	 * @return 				0 if success else errno.
	 */
	int Mtfs::addEntry(internalInode_st *parentInode, std::string name, vector<ident_t> &inodeIds) {
		int ret = 0;

		dirBlock_t dirBlock = dirBlock_t();

		vector<ident_t> blockIdents;

		pool iPool(parentInode->idents.size());

//		if directory is empty add one block
		if (parentInode->inode.dataBlocks.size() == 0) {
			if (0 !=
				(ret = this->dirBlocks->addDirBlock(getRuleInfo(parentInode->inode), blockIdents, this->redundancy))) {
				return ret;
			}

			dirBlock.entries.clear();

			parentInode->inode.atime = this->now();
			parentInode->inode.dataBlocks.push_back(blockIdents);
			for (auto &&ident: parentInode->idents) {
				iPool.schedule(bind(&InodeAcces::putInode, this->inodes, ident, parentInode->inode));
			}
			if (parentInode == this->rootIn)
				this->writeRootInode();

		} else {
//			else get the first block.
			blockIdents = parentInode->inode.dataBlocks.back();

			for (int i = 0; i < blockIdents.size(); ++i) {
				if (0 == (ret = this->dirBlocks->getDirBlock(blockIdents[i], dirBlock)))
					break;
			}

			if (ret != SUCCESS)
				return ret;
		}


//		if block is full, allocate new block.
		if (this->maxEntryPerBlock == dirBlock.entries.size()) {
			ruleInfo_t info = getRuleInfo(parentInode->inode);
			info.lastAccess = this->now();
			if (0 != (ret = this->dirBlocks->addDirBlock(info, blockIdents, this->redundancy))) {
				return ret;
			}

			dirBlock.entries.clear();

			parentInode->inode.atime = this->now();
			parentInode->inode.dataBlocks.push_back(blockIdents);
			for (auto &&ident: parentInode->idents) {
				iPool.schedule(bind(&InodeAcces::putInode, this->inodes, ident, parentInode->inode));
			}
			if (parentInode == this->rootIn)
				this->writeRootInode();
		}


//		write block
		dirBlock.entries.insert(make_pair(name, inodeIds));
		pool wpool(blockIdents.size());
		for (auto &&ident: blockIdents) {
//			TODO try again if block not write.
			wpool.schedule(bind(&DirectoryBlockAccess::putDirBlock, this->dirBlocks, ident, dirBlock));
		}

		return 0;
	}

	/**
	 * @brief Insert inode in system.
	 *
	 * This function get free inode Id and put inode in volumes
	 *
	 * @param[in] inode Inode to put
	 * @param[out] idents Ids to new inode
	 *
	 * @return 0 if SUCCESS else @see satic const vars.
	 */
	int Mtfs::insertInode(const inode_t &inode, vector<ident_t> &idents) {
		int ret;

//		get new inode idents
		if (0 != (ret = this->inodes->addInode(this->getRuleInfo(inode), idents, this->redundancy))) {
			return ret;
		}

#ifdef DEBUG
		cerr << "end addInode" << endl;
#endif

//		write inode in volumes
		{
			pool thPool(idents.size());
			for (auto &&ident: idents) {
				thPool.schedule(bind(&InodeAcces::putInode, this->inodes, ident, inode));
//				this->inodes->putInode(ident, inode);
			}
		}

		return ret;
	}

	/**
	 * @brief dowload or get a directory block.
	 *
	 * This function is create for work in a other thread than worker.
	 *
	 * @param[in] ids 	Block ids
	 * @param[out] q 	Queue
	 * @param[in] queueMutex
	 * @param[in] sem
	 */
	void Mtfs::dlDirBlocks(vector<ident_t> &ids, queue<dirBlock_t> *q, mutex *queueMutex, Semaphore *sem) {
//	TODO gérer le cas ou aucun bloc n'a pu être dl.
		int ret = 1;
		dirBlock_t db = dirBlock_t();

		for (auto &&id: ids) {
			ret = this->dirBlocks->getDirBlock(id, db);
			if (SUCCESS == ret)
				break;
		}

		if (SUCCESS == ret) {
			unique_lock<mutex> lk(*queueMutex);
			q->push(db);
			sem->notify();
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////											UTILS															////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	internalInode_st *Mtfs::getIntInode(fuse_ino_t ino) {
		if (FUSE_ROOT_ID == ino)
			return this->rootIn;
		else
			return (internalInode_st *) ino;
	}

	void Mtfs::getInode(vector<ident_t> &ids, inode_t &inode) {}

	void Mtfs::buildParam(const inode_t &inode, fuse_entry_param &param) {
		param.ino = (fuse_ino_t) &inode;

		param.attr.st_dev = 0;
		param.attr.st_ino = (ino_t) &inode;
		param.attr.st_mode = inode.accesRight;
		param.attr.st_nlink = inode.linkCount;
		param.attr.st_uid = inode.uid;
		param.attr.st_gid = inode.gid;
		param.attr.st_size = inode.size;
		param.attr.st_atim.tv_sec = inode.atime;
		param.attr.st_ctim.tv_sec = inode.atime;
		param.attr.st_mtim.tv_sec = inode.atime;
		param.attr.st_blksize = instance->blockSize;
		param.attr.st_blocks = inode.dataBlocks.size();

		param.generation = 1;
		param.attr_timeout = 1.0;
		param.entry_timeout = 1.0;
	}

	void Mtfs::buildStat(const internalInode_st &inode, struct stat &st) {
		st.st_dev = 0;
		st.st_ino = (__ino_t) &inode;

		st.st_mode = inode.inode.accesRight;
		st.st_nlink = inode.inode.linkCount;
		st.st_uid = inode.inode.uid;
		st.st_gid = inode.inode.gid;
		st.st_size = inode.inode.size;
		time_t time = inode.inode.atime;
		st.st_atim.tv_sec = time;
		st.st_ctim.tv_sec = time;
		st.st_mtim.tv_sec = time;
		st.st_blksize = this->blockSize;
		st.st_blocks = inode.inode.dataBlocks.size();
	}

	ruleInfo_t Mtfs::getRuleInfo(const inode_t &inode) {
		return mtfs::ruleInfo_t(inode.uid, inode.gid, inode.atime);
	}

	inode_t *Mtfs::newInode(const mode_t &mode, const fuse_ctx *ctx) {
		inode_t *inode = new inode_t();

		inode->accesRight = mode;
		inode->uid = ctx->uid;
		inode->gid = ctx->gid;
		if ((mode & S_IFDIR) != 0)
			inode->linkCount = 2;

		return inode;
	}

	uint64_t Mtfs::now() {
		return (uint64_t) time(NULL);
	}


}  // namespace mtfs
