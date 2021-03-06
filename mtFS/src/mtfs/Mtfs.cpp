/**
 * \file Mtfs.cpp
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

#include <mtfs/Mtfs.h>
#include <mtfs/Cache.h>
#include <pluginSystem/PluginManager.h>
#include <fstream>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <boost/filesystem.hpp>
#include <utility>
#include <grp.h>
#include <pwd.h>
#include <utils/Logger.h>

#define GROUPS_TO_SEARCH 30
//TODO compute this value in function of entry size
#define ENTRY_PER_BLOCK 20

namespace mtfs {
	using namespace std;
	using namespace rapidjson;
	using namespace boost::threadpool;

	struct internalInode_st {
		inode_t inode;
		vector<ident_t> idents;
	};

	struct dl_st {
		mutex *fifoMu{nullptr};
		Semaphore *sem{nullptr};
		union {
			queue<pair<string, inode_t>> *inodeQueue;
			queue<dirBlock_t> *dirQueue;
			queue<uint8_t *> *blkQueue;
		} fifo;
		bool end{false};
		mutex *endMu{nullptr};
		pool *dlThPool{nullptr};

		dl_st() {
		}
	};

	struct fd_st {
		enum bt {
			DATA,
			DIR,
		};

		bool firstCall{true};
		bt blockT;
		dl_st blkDl;
		dl_st inoDl;
		boost::thread *blkThr{nullptr};
		boost::thread *inoThr{nullptr};

		fd_st() : blkDl(dl_st()), inoDl(dl_st()) {}

		~fd_st() {
			switch (this->blockT) {
				case DATA:
					delete blkDl.fifo.blkQueue;
					break;
				case DIR:
					delete blkDl.fifo.dirQueue;
					break;
			}
		}
	};


	Mtfs *Mtfs::instance = nullptr;
	pool *Mtfs::threadPool = nullptr;
	string Mtfs::systemName;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////											STATICS															////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Mtfs *Mtfs::getInstance() {
		if (instance == nullptr)
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
		inode.atime = (uint64_t) time(nullptr);
		inode.dataBlocks.clear();

		return true;
	}

	bool Mtfs::start(Document &system, std::string homeDir, string sysName) {
		(void) homeDir;
		systemName = std::move(sysName);

		superblock_t superblock;
		jsonToStruct(system, superblock);

		if (!getInstance()->build(superblock))
			return false;

		auto nbThread = (unsigned int) (thread::hardware_concurrency() * 1.25);
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

		v.SetUint((unsigned int) sb.iCacheSz);
		d.AddMember(rapidjson::StringRef(Mtfs::INODE_CACHE), v, allocator);

		v.SetUint((unsigned int) sb.dCacheSz);
		d.AddMember(rapidjson::StringRef(Mtfs::DIR_CACHE), v, allocator);

		v.SetUint((unsigned int) sb.bCacheSz);
		d.AddMember(rapidjson::StringRef(Mtfs::BLOCK_CACHE), v, allocator);

		v.SetUint((unsigned int) sb.blockSz);
		d.AddMember(rapidjson::StringRef(Mtfs::BLOCK_SIZE_ST), v, allocator);

		v.SetUint((unsigned int) sb.redundancy);
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
		sb.iCacheSz = d[INODE_CACHE].GetUint();

		assert(d.HasMember(DIR_CACHE));
		sb.dCacheSz = d[DIR_CACHE].GetUint();

		assert(d.HasMember(BLOCK_CACHE));
		sb.bCacheSz = d[BLOCK_CACHE].GetUint();

		assert(d.HasMember(BLOCK_SIZE_ST));
		sb.blockSz = d[BLOCK_SIZE_ST].GetUint();

		assert(d.HasMember(REDUNDANCY));
		sb.redundancy = d[REDUNDANCY].GetUint();

		assert(d.HasMember(Rule::MIGRATION));
		sb.migration = d[Rule::MIGRATION].GetInt();

		assert(d.HasMember(Pool::POOLS));
		for (auto &&item :d[Pool::POOLS].GetObject()) {
			string sId = item.name.GetString();
			auto id = (uint32_t) stoul(sId);
			pool_t pool;
			memset(&pool, 0, sizeof(pool_t));
			pool.volumes.clear();

			pool.rule = Rule::buildRule(sb.migration, item.value);

			Pool::jsonToStruct(item.value, pool);

			sb.pools.insert(make_pair(id, pool));
		}

		assert(d.HasMember(ROOT_INODES) && d[ROOT_INODES].IsArray());
		for (auto &&ident: d[ROOT_INODES].GetArray()) {
			ident_t id = ident_t();
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

		this->migratorInfo.poolManager = (PoolManager *) this->blocks;

		this->migratorThr = std::thread(Migrator::main, &this->migratorInfo);

#ifdef DEBUG
		cerr << "[MTFS]: End init" << endl;
#endif
	}

	void Mtfs::destroy(void *userdata) {
		(void) userdata;

		unique_lock<mutex> lk(*this->migratorInfo.endMutex);
		this->migratorInfo.end = true;
		lk.unlock();
		this->migratorInfo.condV.notify_all();

		this->migratorThr.join();

		Acces *iptr = this->inodes;
		Acces *dptr = this->dirBlocks;
		Acces *bptr = this->blocks;

		if (iptr != dptr && iptr != bptr)
			delete this->inodes;

		if (dptr != bptr)
			delete this->dirBlocks;
		delete this->blocks;
	}

	void Mtfs::getAttr(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		(void) fi;

		internalInode_st *inode = this->getIntInode(ino);

		if (nullptr == inode)
			return (void) fuse_reply_err(req, ENOENT);

		struct stat st{};
		this->buildStat(*inode, st);

		fuse_reply_attr(req, &st, this->ATTR_TIMEOUT);
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

		buildStat(*inInode, *attr);
		fuse_reply_attr(req, attr, 1.0);

		pool upThPool(this->SIMULT_UP);
		for (auto &&id: inInode->idents) {
			upThPool.schedule(bind(&Acces::put, this->inodes, id, &inInode->inode, INODE));
		}
	}

	void Mtfs::lookup(fuse_req_t req, fuse_ino_t parent, const string &name) {
		Logger::getInstance()->log("LOOKUP", name, Logger::L_DEBUG);

		internalInode_st *parentInode = this->getIntInode(parent);

//		dl All blocks
		dl_st blkDl = dl_st();
		blkDl.dlThPool = new pool(this->SIMULT_DL);
		blkDl.sem = new Semaphore();
		blkDl.fifoMu = new mutex;
		blkDl.fifo.dirQueue = new queue<dirBlock_t>();
		blkDl.endMu = new mutex;

		this->dlBlocks(parentInode->inode, &blkDl, blockType::DIR_BLOCK, 0);

		vector<ident_t> inodeIds;

		unique_lock<mutex> endLk(*blkDl.endMu, defer_lock);
		unique_lock<mutex> fifoLk(*blkDl.fifoMu, defer_lock);
		lock(endLk, fifoLk);
		while (!(blkDl.end && blkDl.fifo.dirQueue->empty())) {
			endLk.unlock();
			fifoLk.unlock();

			blkDl.sem->wait();

			lock(endLk, fifoLk);
			if (blkDl.end && blkDl.fifo.dirQueue->empty())
				break;
			endLk.unlock();

			dirBlock_t block = blkDl.fifo.dirQueue->front();
			blkDl.fifo.dirQueue->pop();
			fifoLk.unlock();

//			if entry is find
			if (block.entries.end() != block.entries.find(name)) {
				blkDl.dlThPool->clear();
				inodeIds = block.entries[name];

				lock(endLk, fifoLk);
				break;
			}

			lock(endLk, fifoLk);
		}
		endLk.unlock();
		fifoLk.unlock();


//		TODO Doit etre un pointeur car lookup avant open donc inode doit etre en memoire.
		auto *inode = new internalInode_st();
		inode->idents = inodeIds;
		int ret;

		if (inodeIds.empty()) {
			fuse_reply_err(req, ENOENT);
			return;
		}

		do {
			ret = this->inodes->get(inodeIds.back(), &inode->inode, blockType::INODE);
			inodeIds.pop_back();
			if (inodeIds.empty())
				break;
		} while (0 != ret);

//		Send reply to fuse
		if (0 != ret)
			fuse_reply_err(req, ret);
		else {
			fuse_entry_param param = fuse_entry_param();
			this->buildParam(*inode, param);
			fuse_reply_entry(req, &param);
		}

//		free all datas;
		blkDl.dlThPool->wait();

		delete (blkDl.dlThPool);
		delete (blkDl.sem);
		delete (blkDl.fifoMu);
		delete (blkDl.endMu);

		while (!blkDl.fifo.dirQueue->empty())
			blkDl.fifo.dirQueue->pop();

		delete (blkDl.fifo.dirQueue);
	}

	void Mtfs::mknod(fuse_req_t req, fuse_ino_t parent, const string &name, mode_t mode, dev_t rdev) {
		(void) rdev;

		int ret;

		internalInode_st *parentInode = this->getIntInode(parent);

//		Create and write new inode
		internalInode_st *inode = this->newInode(mode, fuse_req_ctx(req));
		vector<ident_t> inodeIdents;
		if (0 != (ret = this->insertInode(inode->inode, inodeIdents))) {
			delete inode;
			fuse_reply_err(req, ret);
			return;
		}

		pool mPool(SIMULT_UP);
		this->initMetas(*parentInode, inodeIdents, blockType::INODE, &mPool);

//			Add entry in dir
		if (0 != (ret = this->addEntry(parentInode, name, inodeIdents))) {
			delete inode;
			fuse_reply_err(req, ret);
			return;
		}

//			reply to fuse
		fuse_entry_param param{};
		memset(&param, 0, sizeof(fuse_entry_param));

		this->buildParam(*inode, param);

		fuse_reply_entry(req, &param);
	}

	void Mtfs::mkdir(fuse_req_t req, fuse_ino_t ino, const string &name, mode_t mode) {
		int ret;
		internalInode_st *parentInode = this->getIntInode(ino);

//		Create and write new inode
		internalInode_st *inode = this->newInode(mode, fuse_req_ctx(req));
		inode->inode.accesRight |= S_IFDIR;
		vector<ident_t> inodeIdents;
		if (0 != (ret = this->insertInode(inode->inode, inodeIdents))) {
			delete inode;
			fuse_reply_err(req, ret);
			return;
		}

		pool mPool(SIMULT_UP);
		this->initMetas(*parentInode, inodeIdents, blockType::INODE, &mPool);

//			Add entry in dir
		if (0 != (ret = this->addEntry(parentInode, name, inodeIdents))) {
			delete inode;
			fuse_reply_err(req, ret);
			return;
		}

//			reply to fuse
		fuse_entry_param param{};
		memset(&param, 0, sizeof(fuse_entry_param));

		this->buildParam(*inode, param);

		fuse_reply_entry(req, &param);
	}

	void Mtfs::unlink(fuse_req_t req, fuse_ino_t parent, const string &name) {
		internalInode_st *parentInode = this->getIntInode(parent);

		int ret;
		if (SUCCESS != (ret = this->doUnlink(parentInode, name)))
			fuse_reply_err(req, ret);
		else
			fuse_reply_err(req, SUCCESS);
	}

	void Mtfs::rmdir(fuse_req_t req, fuse_ino_t parent, const string &name) {
		(void) parent, name;

		fuse_reply_err(req, ENOSYS);
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
				auto bufsize = (size_t) sysconf(_SC_GETPW_R_SIZE_MAX);
				if (bufsize == -1)
					bufsize = 16384;

				auto *buf = (char *) malloc(bufsize * sizeof(char));
				if (nullptr == buf) {
					fuse_reply_err(req, ENOMEM);
					free(buf);
					return;
				}

				struct passwd pwd{}, *result;
				int s = getpwuid_r(context->uid, &pwd, buf, bufsize, &result);
				if (nullptr == result) {
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
				auto *groups = (gid_t *) malloc(ngroups * sizeof(gid_t));
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
			fuse_reply_err(req, SUCCESS);
		}
	}

	void Mtfs::open(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		(void) ino;
//		internalInode_st *inode = getIntInode(ino);

//		if (!inode->inode.dataBlocks.empty()) {
//			uint8_t *firstBlock = (uint8_t *) malloc(this->blockSize * sizeof(uint8_t));
//
//			this->blocks->get(inode->inode.dataBlocks.front().front(), &firstBlock, blockType::DATA_BLOCK);
//			fi->fh = (uint64_t) firstBlock;
//		} else
//			fi->fh = 0;A

		auto *fd = new fd_st();
		fd->blockT = fd->DATA;
		fd->blkDl.dlThPool = new pool(this->SIMULT_DL);
		fd->blkDl.sem = new Semaphore();
		fd->blkDl.fifoMu = new mutex();
		fd->blkDl.endMu = new mutex();
		fd->blkDl.fifo.blkQueue = new queue<uint8_t *>();
		fd->inoDl.dlThPool = new pool(this->SIMULT_UP);

		fi->fh = (uint64_t) fd;

		fuse_reply_open(req, fi);
	}

	void Mtfs::release(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		(void) ino;
//		internalInode_st *intIno = this->getIntInode(ino);

		delete (uint8_t *) fi->fh;
		fi->fh = 0;

		fuse_reply_err(req, SUCCESS);
	}

	void Mtfs::opendir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {

		internalInode_st *intInode = this->getIntInode(ino);

		auto *fd = new fd_st();
		fd->blockT = fd->DIR;
		fd->blkDl.dlThPool = new pool(this->SIMULT_DL);
		fd->blkDl.sem = new Semaphore();
		fd->blkDl.fifoMu = new mutex();
		fd->blkDl.endMu = new mutex();
		fd->blkDl.fifo.dirQueue = new queue<dirBlock_t>();

		int i = 0;
		for (auto &&blks: intInode->inode.dataBlocks) {

//			bind(&Mtfs::dlDirBlocks, this, blks, &blkQueue, &queueMutex, &semaphore));
			fd->blkDl.dlThPool->schedule(
					bind(&Mtfs::dlDirBlocks, this, blks, fd->blkDl.fifo.dirQueue, fd->blkDl.fifoMu, fd->blkDl.sem));

			if (this->INIT_DL <= i)
				break;
		}

		fi->fh = (uint64_t) fd;

		fuse_reply_open(req, fi);
//		fuse_reply_err(req, ENOSYS);
	}

	void Mtfs::readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) {
		this->doReaddir(req, ino, size, off, fi, false);
	}

	void Mtfs::readdirplus(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) {
		this->doReaddir(req, ino, size, off, fi, true);
	}

	void Mtfs::releasedir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		(void) ino;

		auto *fd = (fd_st *) fi->fh;
		delete fd;
		fi->fh = 0;

		fuse_reply_err(req, SUCCESS);
	}

	void Mtfs::write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, fuse_file_info *fi) {
		(void) ino, buf, size, off, fi;
//		internalInode_st *inode = this->getIntInode(ino);
//
//		int startBlock = (int) (off / this->blockSize);
//
//		fuse_reply_write(req, size);
		fuse_reply_err(req, ENOSYS);
	}

	void Mtfs::write_buf(fuse_req_t req, fuse_ino_t ino, fuse_bufvec *bufv, off_t off, fuse_file_info *fi) {
		int ret = 0;

		auto *fd = (fd_st *) fi->fh;

		internalInode_st *inode = this->getIntInode(ino);

		size_t size = bufv->buf[0].size;
		auto firstBlock = (int) (off / this->blockSize);
		auto lastBlock = (int) ((off + size - 1) / this->blockSize);

		ruleInfo_t newBlockInfo = ruleInfo_st();
		newBlockInfo.gid = inode->inode.gid;
		newBlockInfo.uid = inode->inode.uid;
		newBlockInfo.lastAccess = this->now();

		size_t rem = size;
		size_t write = 0;

		pool metasTp(SIMULT_UP);

		vector<uint64_t> toFree;

		for (int blockToWrite = firstBlock; blockToWrite <= lastBlock; ++blockToWrite) {
			auto *block = (uint8_t *) calloc(sizeof(uint8_t), this->blockSize);
			memset(block, 0, this->blockSize);

			vector<ident_t> blockIdents;
			if (inode->inode.dataBlocks.empty() || inode->inode.dataBlocks.size() <= blockToWrite) {
				if (SUCCESS !=
					(ret = this->blocks->add(newBlockInfo, blockIdents, blockType::DATA_BLOCK,
											 (const int) this->redundancy))) {
					fuse_reply_err(req, ret);
					return;
				}

				this->initMetas(*inode, blockIdents, blockType::DATA_BLOCK, &metasTp);

				inode->inode.dataBlocks.push_back(blockIdents);
			} else {
				blockIdents = inode->inode.dataBlocks[blockToWrite];

				for (auto &&id: blockIdents) {
					ret = this->blocks->get(id, block, blockType::DATA_BLOCK);
					if (SUCCESS == ret)
						break;
				}
				if (SUCCESS != ret)
					return (void) fuse_reply_err(req, ret);
			}

			size_t startPos = blockToWrite == firstBlock ? off % this->blockSize : 0;
			size_t endPos = (blockToWrite == lastBlock ? rem % (this->blockSize + 1) : this->blockSize) - startPos;

			memcpy(block + startPos, bufv->buf->mem, endPos);
			write += endPos;
			rem -= endPos;

			for (auto &&id: blockIdents) {
				fd->blkDl.dlThPool->schedule(bind(&Acces::put, this->blocks, id, block, blockType::DATA_BLOCK));
			}

			toFree.push_back((uint64_t) block);
		}

		auto *data = (uint8_t *) malloc(size * sizeof(uint8_t));
		memcpy(data, bufv->buf[0].mem, bufv->buf[0].size);

		inode->inode.size = off + size;

//		TODO supprimer les block en trop.
		vector<ident_t> toDel;
		if (inode->inode.dataBlocks.size() > lastBlock + 1) {
			for (auto &dataBlock : inode->inode.dataBlocks) {
				for (auto &&id: dataBlock) {
					toDel.push_back(id);
				}
			}

			inode->inode.dataBlocks.erase(inode->inode.dataBlocks.begin() + lastBlock + 1,
										  inode->inode.dataBlocks.end());
		}

		for (auto &&id: inode->idents) {
			fd->inoDl.dlThPool->schedule(bind(&Acces::put, this->inodes, id, &inode->inode, blockType::INODE));
		}

		fd->blkDl.dlThPool->wait();
		fd->inoDl.dlThPool->wait();

		fuse_reply_write(req, write);

		pool delPool(this->redundancy);
		for (auto &&id: toDel) {
			delPool.schedule(bind(&Acces::del, this->blocks, id, blockType::DATA_BLOCK));
		}

		for (auto &&ptr: toFree) {
			free((uint8_t *) ptr);
		}
	}

	void Mtfs::read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) {
		(void) fi;
		internalInode_st *inode = this->getIntInode(ino);


		size = min(size, inode->inode.size);
		int firstBlock, lastBlock;
		firstBlock = (int) (off / this->blockSize);
		lastBlock = (int) ((off + size - 1) / this->blockSize);

		size_t rem = size;
		size_t read = 0;

		char *buf;
		buf = new char[size];
		char *p = buf;

		for (int blockToRead = firstBlock; blockToRead <= lastBlock; blockToRead++) {
			uint8_t block[this->blockSize];
			if (SUCCESS != this->blocks->get(inode->inode.dataBlocks[blockToRead][0], block, blockType::DATA_BLOCK)) {
				delete[]buf;
				fuse_reply_err(req, EAGAIN);

				return;
			}

			size_t startPos = blockToRead == firstBlock ? off % this->blockSize : 0;
			size_t endPos = blockToRead == lastBlock ? rem % (this->blockSize + 1) : this->blockSize;

			size_t toCopy = endPos - startPos;
			memcpy(p, block + startPos, toCopy);

			rem -= toCopy;
			read += toCopy;
			p += toCopy;
		}

		fuse_reply_buf(req, buf, read);
		delete[]buf;
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

	/**
	 * Get the root inode
	 *
	 * @return the inode
	 */
	inode_t Mtfs::getRootInode() {
		return this->rootIn->inode;
//		return this->rootInode;
	}

	/**
	 * Build MTFS system
	 *
	 * @param superblock
	 * @return
	 */
	bool Mtfs::build(const superblock_t &superblock) {

		this->redundancy = superblock.redundancy;
		this->blockSize = superblock.blockSz;
		this->rootIn->idents = superblock.rootInodes;

		pluginSystem::PluginManager *manager = pluginSystem::PluginManager::getInstance();

		PoolManager *poolManager;
		poolManager = new PoolManager();
		for (auto &&poolSt: superblock.pools) {
			Pool *pool;
			pool = new Pool(this->blockSize);

			for (auto volSt: poolSt.second.volumes) {
				volSt.second.params.insert(make_pair("home", MTFS_PLUGIN_HOME));
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

	/**
	 * Read the root inode
	 */
	void Mtfs::readRootInode() {
		string filename = string(MTFS_INSTALL_DIR) + "/" + systemName + "/root.json";
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

		assert(d.HasMember(IN_BLOCKS));
		for (auto &&item: d[IN_BLOCKS].GetArray()) {
			vector<ident_t> blocksRedundancy;
			for (auto &&block: item.GetArray()) {
				ident_t ident = ident_t();

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

	/**
	 * Write the root inode
	 */
	void Mtfs::writeRootInode() {
		Document d;

		this->rootIn->inode.toJson(d);

		StringBuffer sb;
		PrettyWriter<StringBuffer> pw(sb);
		d.Accept(pw);

		string filename = string(MTFS_INSTALL_DIR) + "/" + systemName + "/root.json";
		ofstream rootFile(filename);
		rootFile << sb.GetString() << endl;
		rootFile.close();
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
		pool mPool(SIMULT_UP);

//		if directory is empty add one block
		if (parentInode->inode.dataBlocks.empty()) {
			if (0 !=
				(ret = this->dirBlocks->add(getRuleInfo(parentInode->inode), blockIdents, blockType::DIR_BLOCK,
											this->redundancy))) {
				return ret;
			}

			dirBlock.entries.clear();

			this->initMetas(*parentInode, blockIdents, blockType::DIR_BLOCK, &mPool);

			parentInode->inode.atime = this->now();
			parentInode->inode.dataBlocks.push_back(blockIdents);
			for (auto &&ident: parentInode->idents) {
				iPool.schedule(bind(&Acces::put, this->inodes, ident, &parentInode->inode, blockType::INODE));
			}
			if (parentInode == this->rootIn)
				this->writeRootInode();

		} else {
//			else get the first block.
			blockIdents = parentInode->inode.dataBlocks.back();

			for (const auto &blockIdent : blockIdents) {
				if (0 == (ret = this->dirBlocks->get(blockIdent, &dirBlock, blockType::DIR_BLOCK)))
					break;
			}

			if (ret != SUCCESS)
				return ret;
		}


//		if block is full, allocate new block.
		if (this->maxEntryPerBlock == dirBlock.entries.size()) {
			ruleInfo_t info = getRuleInfo(parentInode->inode);
			info.lastAccess = this->now();
			if (0 != (ret = this->dirBlocks->add(info, blockIdents, blockType::DIR_BLOCK, this->redundancy))) {
				return ret;
			}

			this->initMetas(*parentInode, blockIdents, blockType::DIR_BLOCK, &mPool);

			dirBlock.entries.clear();

			parentInode->inode.atime = this->now();
			parentInode->inode.dataBlocks.push_back(blockIdents);
			for (auto &&ident: parentInode->idents) {
				iPool.schedule(bind(&Acces::put, this->inodes, ident, &parentInode->inode, blockType::INODE));
			}
			if (parentInode == this->rootIn)
				this->writeRootInode();
		}


//		write block
		dirBlock.entries.insert(make_pair(name, inodeIds));
		pool wpool(blockIdents.size());
		for (auto &&ident: blockIdents) {
//			TODO try again if block not write.
			wpool.schedule(bind(&Acces::put, this->dirBlocks, ident, &dirBlock, blockType::DIR_BLOCK));
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
		if (0 != (ret = this->inodes->add(this->getRuleInfo(inode), idents, blockType::INODE, this->redundancy))) {
			return ret;
		}

//		write inode in volumes
		{
			pool thPool(idents.size());
			for (auto &&ident: idents) {
				thPool.schedule(bind(&Acces::put, this->inodes, ident, &inode, blockType::INODE));
			}
		}

		return ret;
	}

	/**
	 * Download all blocks in inode
	 *
	 * @param inode Inode who containe the blocks
	 * @param dlSt Download struct
	 * @param type Type of block to dowload DIR_BLOCK | DATA_BLOCK
	 * @param firstBlockIdx Index of first block to download
	 */
	void Mtfs::dlBlocks(const inode_t &inode, dl_st *dlSt, const blockType type, const int firstBlockIdx) {
//		TODO implements

		if (firstBlockIdx < inode.dataBlocks.size()) {
			for (auto idents = inode.dataBlocks.begin() + firstBlockIdx; idents != inode.dataBlocks.end(); idents++) {
				switch (type) {
					case DIR_BLOCK:
						dlSt->dlThPool->schedule(
								bind(&Mtfs::dlDirBlocks, this, *idents, dlSt->fifo.dirQueue, dlSt->fifoMu, dlSt->sem));
						break;
					case DATA_BLOCK:
						break;
					default:
//					TODO Log error
						break;
				}
			}
		}

		dlSt->dlThPool->wait();

		unique_lock<mutex> endLk(*dlSt->endMu);
		dlSt->end = true;
		endLk.unlock();
		dlSt->sem->notify();
	}

	/**
	 * @brief download or get a directory block.
	 *
	 * This function is create for work in a other thread than worker.
	 *
	 * @param[in] ids 	Block ids
	 * @param[out] q 	Queue
	 * @param[in] queueMutex
	 * @param[in] sem
	 */
	void Mtfs::dlDirBlocks(vector<ident_t> &ids, queue<dirBlock_t> *q, mutex *queueMutex, Semaphore *sem) {
//	TODO gerer le cas ou aucun bloc n'a pu etre dl.
		int ret = 1;
		dirBlock_t db = dirBlock_t();

		for (auto &&id: ids) {
			ret = this->dirBlocks->get(id, &db, blockType::DIR_BLOCK);
			if (SUCCESS == ret)
				break;
		}

		if (SUCCESS == ret) {
			unique_lock<mutex> lk(*queueMutex);
			q->push(db);
			sem->notify();
		}
	}

	/**
	 * Download all inodes from dir entries.
	 *
	 * @param src dl dir entries struct
	 * @param dst dl inode struct
	 */
	void Mtfs::dlInodes(dl_st *src, dl_st *dst) {
		pool inodesPool(this->SIMULT_DL);
		unique_lock<mutex> srcEndLk(*src->endMu, defer_lock);
		unique_lock<mutex> srcFifoLk(*src->fifoMu, defer_lock);
		std::lock(srcEndLk, srcFifoLk);
		while (!(src->end && src->fifo.dirQueue->empty())) {
			srcEndLk.unlock();
			srcFifoLk.unlock();

			src->sem->wait();

			std::lock(srcEndLk, srcFifoLk);
			if (src->end && src->fifo.dirQueue->empty())
				break;
			srcEndLk.unlock();

			dirBlock_t dirBlock = src->fifo.dirQueue->front();
			src->fifo.dirQueue->pop();
			srcFifoLk.unlock();

			for (auto &&entry: dirBlock.entries) {
				inodesPool.schedule(
						bind(&Mtfs::dlInode, this, entry.second, dst->fifo.inodeQueue, dst->fifoMu, dst->sem,
							 entry.first));
			}

			std::lock(srcEndLk, srcFifoLk);
		}
		srcEndLk.unlock();
		srcFifoLk.unlock();

		inodesPool.wait();

		unique_lock<mutex> dstEndLock(*dst->endMu);
		dst->end = true;
		dstEndLock.unlock();

		dst->sem->notify();
	}

	/**
	 * Download inodes
	 *
	 * @param ids
	 * @param queue
	 * @param queueMutex
	 * @param sem
	 * @param key
	 */
	void
	Mtfs::dlInode(vector<ident_t> &ids, queue<pair<string, inode_t>> *queue, mutex *queueMutex, Semaphore *sem,
				  string &key) {
		int ret = 1;
		inode_t in = inode_t();

		for (auto &&id: ids) {
			ret = this->inodes->get(id, &in, blockType::INODE);
			if (SUCCESS == ret)
				break;
		}

		if (SUCCESS == ret) {
			unique_lock<mutex> lk(*queueMutex);
			queue->push(make_pair(key, in));
			sem->notify();
		}
	}

	internalInode_st *Mtfs::getIntInode(fuse_ino_t ino) {
		if (FUSE_ROOT_ID == ino)
			return this->rootIn;

		return (internalInode_st *) ino;
	}

	void Mtfs::doReaddir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi, const bool &plus) {
		(void) off;
		internalInode_st *inode = this->getIntInode(ino);
		fd_st *fd;
		fd = (fd_st *) fi->fh;

		char *buf, *p;
		size_t rem, currSize;

		buf = (char *) calloc(size, 1);
		if (nullptr == buf)
			return (void) fuse_reply_err(req, ENOMEM);

		p = buf;
		rem = size;
		currSize = 0;

		if (fd->firstCall) {
			fd->firstCall = false;
			size_t entrySize = this->dirBufAdd(req, p, currSize, ".", *inode, plus);
			p += entrySize;
			rem -= entrySize;
			currSize += entrySize;

//			Dl other blocks and inodes
			int idx = this->INIT_DL;
			fd->blkThr = new boost::thread(
					bind(&Mtfs::dlBlocks, this, inode->inode, &fd->blkDl, blockType::DIR_BLOCK, idx));

			fd->inoDl.fifoMu = new mutex();
			fd->inoDl.endMu = new mutex();
			fd->inoDl.sem = new Semaphore();
			fd->inoDl.fifo.inodeQueue = new queue<pair<string, inode_t>>();

			fd->inoThr = new boost::thread(bind(&Mtfs::dlInodes, this, &fd->blkDl, &fd->inoDl));
		}

		unique_lock<mutex> inoEndLock(*fd->inoDl.endMu, defer_lock);
		unique_lock<mutex> inoFifoLock(*fd->inoDl.fifoMu, defer_lock);
		lock(inoEndLock, inoFifoLock);
		while (!(fd->inoDl.end && fd->inoDl.fifo.inodeQueue->empty())) {
			inoEndLock.unlock();
			inoFifoLock.unlock();

			fd->inoDl.sem->wait();

			internalInode_st *intInode;
			intInode = new internalInode_st();
//			shared_ptr<internalInode_st> intInode(new internalInode_st());
			lock(inoEndLock, inoFifoLock);
			if (fd->inoDl.end && fd->inoDl.fifo.inodeQueue->empty())
				break;
			inoEndLock.unlock();

			string entryName = fd->inoDl.fifo.inodeQueue->front().first;
			intInode->inode = fd->inoDl.fifo.inodeQueue->front().second;
			fd->inoDl.fifo.inodeQueue->pop();
			inoFifoLock.unlock();


			size_t entrySize = this->dirBufAdd(req, p, currSize, entryName, *intInode, plus);

			if (entrySize > rem) {
				lock(inoEndLock, inoFifoLock);
				break;
			}

//			p += entrySize;
			rem -= entrySize;
			currSize += entrySize;

			lock(inoEndLock, inoFifoLock);
			break;
		}
		inoEndLock.unlock();
		inoFifoLock.unlock();
		fuse_reply_buf(req, buf, size - rem);
	}

	size_t Mtfs::dirBufAdd(fuse_req_t &req, char *buf, size_t &currentSize, std::string name, internalInode_st &inode,
						   const bool &plus) {
		size_t entSize = 0;

		if (plus) {
			fuse_entry_param *p;
			p = new fuse_entry_param();
			buildParam(inode, *p);

			entSize += fuse_add_direntry_plus(req, nullptr, 0, name.c_str(), nullptr, 0);
			fuse_add_direntry_plus(req, buf, entSize, name.c_str(), p, currentSize + entSize);
		} else {
			struct stat stbuf{};
			memset(&stbuf, 0, sizeof(stbuf));
			buildStat(inode, stbuf);

			entSize += fuse_add_direntry(req, nullptr, 0, name.c_str(), nullptr, 0);
			fuse_add_direntry(req, buf, entSize, name.c_str(), &stbuf, currentSize + entSize);
		}
		return entSize;
	}

	void Mtfs::buildParam(const internalInode_st &inode, fuse_entry_param &param) {
		param.ino = (fuse_ino_t) &inode;

		this->buildStat(inode, param.attr);

		param.generation = 1;
		param.attr_timeout = 0.0;
		param.entry_timeout = 0.0;
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
		return {inode.uid, inode.gid, inode.atime};
	}

	internalInode_st *Mtfs::newInode(const mode_t &mode, const fuse_ctx *ctx) {
		internalInode_st *inode;
		inode = new internalInode_st();

		inode->inode.accesRight = mode;
		inode->inode.uid = ctx->uid;
		inode->inode.gid = ctx->gid;
		if ((mode & S_IFDIR) != 0)
			inode->inode.linkCount = 2;

		return inode;
	}

	uint64_t Mtfs::now() {
		return (uint64_t) time(nullptr);
	}

	/**
	 * Init metas blocs
	 *
	 * @param parentInode
	 * @param ids
	 * @param type
	 * @param thPool
	 */
	void Mtfs::initMetas(const internalInode_st &parentInode, const vector<ident_t> &ids, const blockType &type,
						 boost::threadpool::pool *thPool) {
		blockInfo_t metas = blockInfo_t();
		metas.lastAccess = (uint64_t) time(nullptr);
		metas.referenceId = parentInode.idents;

		for (auto &&id :ids) {
			metas.id = id;

			switch (type) {
				case INODE:
					if (nullptr != thPool)
						thPool->schedule(bind(&Acces::putMetas, this->inodes, id, metas, type));
					else
						this->inodes->putMetas(id, metas, type);
					break;
				case DIR_BLOCK:
					if (nullptr != thPool)
						thPool->schedule(bind(&Acces::putMetas, this->dirBlocks, id, metas, type));
					else
						this->dirBlocks->putMetas(id, metas, type);
					break;
				case DATA_BLOCK:
					if (nullptr != thPool)
						thPool->schedule(bind(&Acces::putMetas, this->blocks, id, metas, type));
					else
						this->blocks->putMetas(id, metas, type);
					break;
				default:
					break;
			}
		}
	}

	int Mtfs::doUnlink(internalInode_st *parent, const std::string name) {

		for (auto &&dBlkIds :parent->inode.dataBlocks) {
			dirBlock_t dBlk = dirBlock_t();

			this->dirBlocks->get(dBlkIds.front(), &dBlk, blockType::DIR_BLOCK);

			if (dBlk.entries.end() != dBlk.entries.find(name))
				return this->delEntry(dBlkIds, dBlk, name);
		}
		return ENOENT;
	}

	int Mtfs::delEntry(std::vector<ident_t> &ids, dirBlock_t &blk, const std::string &name) {
		vector<ident_t> inodeIds = blk.entries.find(name)->second;

		inode_t inode = inode_t();
		this->inodes->get(inodeIds.front(), &inode, blockType::INODE);

		if (0 < (inode.accesRight & S_IFDIR))
			return EISDIR;

		blk.entries.erase(name);
		pool upPool(SIMULT_UP);

		for (auto &&id :ids) {
			upPool.schedule(bind(&Acces::put, this->dirBlocks, id, &blk, blockType::DIR_BLOCK));
		}

		for (auto &&dataBlkIds :inode.dataBlocks) {
			for (auto &&blkId :dataBlkIds) {
				this->blocks->del(blkId, blockType::DATA_BLOCK);
			}
		}

		for (auto &&inodeId :inodeIds) {
			this->inodes->del(inodeId, blockType::INODE);
		}

		return 0;
	}


}  // namespace mtfs
