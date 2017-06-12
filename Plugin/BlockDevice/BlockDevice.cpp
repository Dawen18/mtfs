#include <iostream>
#include <sys/mount.h>
#include <map>
#include <sys/stat.h>
#include <fcntl.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <memory>
#include <mtfs/Mtfs.h>
#include <mtfs/Pool.h>

#include "BlockDevice.h"

//#define DEBUG

using namespace std;
using namespace rapidjson;
using namespace mtfs;

namespace pluginSystem {
	BlockDevice::BlockDevice() : nextFreeBlock(0), nextFreeDirBlock(0), nextFreeInode(1) {
		srand((unsigned int) time(NULL));
		this->freeBlocks.clear();
		this->freeDirBlocks.clear();
		this->freeInodes.clear();
	}

	BlockDevice::~BlockDevice() {
		this->freeBlocks.clear();
		this->freeDirBlocks.clear();
		this->freeInodes.clear();
	}

	string BlockDevice::getName() {
		return "block";
	}

	bool BlockDevice::attach(std::map<string, string> params) {
		if (params.find("devicePath") == params.end() || params.find("home") == params.end() ||
			params.find("fsType") == params.end())
			return false;

		string home;
		home = params["home"];
		if (*home.end() != '/')
			home += '/';
		string parentDir = home + "BlockDevices/";
		string bs = params["blockSize"];
		this->blockSize = stoi(params.at("blockSize"));
		this->fsType = params["fsType"];
		this->devicePath = params["devicePath"];
		this->mountpoint = parentDir + this->devicePath.substr(this->devicePath.find('/', 1) + 1);

#ifdef DEBUG
		//		cout << "attach to " << this->mountpoint;
#endif

		if (!dirExists(parentDir)) {
			if (mkdir(parentDir.c_str(), 0700) != 0) {
				logError("mkdir error " + parentDir + " " + strerror(errno));
				return false;
			}
		}

		if (!dirExists(this->mountpoint)) {
			if (mkdir(this->mountpoint.c_str(), 0700) != 0) {
				logError("mkdir error " + this->mountpoint + " " + strerror(errno));
				return false;
			}
		}

#ifndef DEBUG
		//		Mount device
		mount(this->devicePath.c_str(), this->mountpoint.c_str(), this->fsType.c_str(), 0, NULL);
#endif

		initDirHierarchie();

		initInodes();
		initDirBlocks();
		initBlocks();

		return true;
	}

	bool BlockDevice::detach() {
		writeMetas();

#ifndef DEBUG
		if (umount(this->mountpoint.c_str()) != 0) {
			cerr << "ERROR Failed to umount: " << this->mountpoint << endl;
			cerr << "reason: " << strerror(errno) << " [" << errno << "]" << endl;
			return false;
		}
#endif

		return true;
	}

	int BlockDevice::addInode(uint64_t *inodeId) {
		unique_lock<mutex> lk(this->inodeMutex);
		if (this->freeInodes.size() == 0) {
			*inodeId = this->nextFreeInode;
			this->nextFreeInode++;
		} else {
			*inodeId = this->freeInodes[0];
			this->freeInodes.erase(freeInodes.begin());
		}
		lk.unlock();

		string filename = this->mountpoint + "/" + INODES_DIR + "/" + to_string(*inodeId);

		return createFile(filename);
	}

	int BlockDevice::delInode(const uint64_t &inodeId) {
		unique_lock<mutex> lk(this->inodeMutex);
		if (this->nextFreeInode - 1 == inodeId)
			this->nextFreeInode--;
		else {
			this->freeInodes.push_back(inodeId);
		}
		lk.unlock();

		string filename = this->mountpoint + "/" + INODES_DIR + "/" + to_string(inodeId);

		return deleteFile(filename);
	}

	int BlockDevice::getInode(const uint64_t &inodeId, mtfs::inode_st &inode) {
		string filename = this->mountpoint + "/" + INODES_DIR + "/" + to_string(inodeId);
		ifstream file(filename);
		if (!file.is_open())
			return errno != 0 ? errno : ENOENT;

		IStreamWrapper wrapper(file);

		Document d;
		d.ParseStream(wrapper);

		assert(d.IsObject());
		assert(d.HasMember(IN_MODE));
		assert(d.HasMember(IN_UID));
		assert(d.HasMember(IN_GID));
		assert(d.HasMember(IN_SIZE));
		assert(d.HasMember(IN_LINKS));
		assert(d.HasMember(IN_ACCESS));
		assert(d.HasMember(IN_REFF));
		assert(d.HasMember(IN_BLOCKS));

		inode.accesRight = (uint16_t) d[IN_MODE].GetUint();
		inode.uid = (uint16_t) d[IN_UID].GetUint();
		inode.gid = (uint16_t) d[IN_GID].GetUint();
		inode.size = d[IN_SIZE].GetUint64();
		inode.linkCount = (uint8_t) d[IN_LINKS].GetUint();
		inode.atime = d[IN_ACCESS].GetUint64();

		const Value &referenceArray = d[IN_REFF];
		assert(referenceArray.IsArray());
		inode.referenceId.clear();
		for (auto &v : referenceArray.GetArray()) {
			ident_t ident;

			assert(v.IsObject());
			assert(v.HasMember(ID_POOL));
			assert(v.HasMember(ID_VOLUME));
			assert(v.HasMember(ID_ID));

			ident.poolId = (uint16_t) v[ID_POOL].GetUint();
			ident.volumeId = (uint16_t) v[ID_VOLUME].GetUint();
			ident.id = v[ID_ID].GetUint64();

			inode.referenceId.push_back(ident);
		}

		const Value &dataArray = d[IN_BLOCKS];
		assert(dataArray.IsArray());
		inode.dataBlocks.clear();
		for (auto &a : dataArray.GetArray()) {
			vector<ident_t> redundancy;

			assert(a.IsArray());
			for (auto &v : a.GetArray()) {
				ident_t ident;

				assert(v.IsObject());
				assert(v.HasMember(ID_POOL));
				assert(v.HasMember(ID_VOLUME));
				assert(v.HasMember(ID_ID));

				ident.poolId = (uint16_t) v[ID_POOL].GetUint();
				ident.volumeId = (uint16_t) v[ID_VOLUME].GetUint();
				ident.id = v[ID_ID].GetUint64();

				redundancy.push_back(ident);
			}

			inode.dataBlocks.push_back(redundancy);
		}

		return this->SUCCESS;
	}

	int BlockDevice::putInode(const uint64_t &inodeId, const inode_st &inode) {
		Document d;
		d.SetObject();
		Document::AllocatorType &alloc = d.GetAllocator();

		inode.toJson(d);

		StringBuffer sb;
		PrettyWriter<StringBuffer> pw(sb);
		d.Accept(pw);

		string filename = this->mountpoint + "/" + BlockDevice::INODES_DIR + "/" + to_string(inodeId);
		ofstream inodeFile(filename);
		inodeFile << sb.GetString() << endl;
		inodeFile.close();

		return this->SUCCESS;
	}

	int BlockDevice::addDirBlock(uint64_t *id) {
		unique_lock<mutex> lk(this->dirBlockMutex);
		if (this->freeDirBlocks.size() == 0) {
			*id = this->nextFreeDirBlock;
			this->nextFreeDirBlock++;
		} else {
			*id = this->freeDirBlocks.front();
			iter_swap(this->freeDirBlocks.begin(), this->freeDirBlocks.end());
			this->freeDirBlocks.pop_back();
		}
		lk.unlock();

		string filename = this->mountpoint + "/" + DIR_BLOCKS_DIR + "/" + to_string(*id);

		return createFile(filename);
	}

	int BlockDevice::delDirBlock(const uint64_t &id) {
		unique_lock<mutex> lk(this->dirBlockMutex);
		if (id == this->nextFreeDirBlock - 1)
			this->nextFreeDirBlock--;
		else
			this->freeDirBlocks.push_back(id);
		lk.unlock();

		string filename = this->mountpoint + "/" + DIR_BLOCKS_DIR + "/" + to_string(id);
		return deleteFile(filename);
	}

	int BlockDevice::getDirBlock(const uint64_t &id, dirBlock_t &block) {
		string filename = this->mountpoint + "/" + DIR_BLOCKS_DIR + "/" + to_string(id);
		ifstream file(filename);
		if (!file.is_open())
			return errno != 0 ? errno : ENOENT;


		IStreamWrapper wrapper(file);

		Document d;
		d.ParseStream(wrapper);

		assert(d.IsObject());

		for (auto &&item: d.GetObject()) {
			assert(item.value.IsArray());

			vector<ident_t> ids;

			for (auto &&ident: item.value.GetArray()) {
				ident_t i;
				i.fromJson(ident);
				ids.push_back(i);
			}
			block.entries.insert(make_pair(item.name.GetString(), ids));
		}
		return this->SUCCESS;
	}

	int BlockDevice::putDirBlock(const uint64_t &id, const dirBlock_t &block) {
		Document d;
		d.SetObject();

		Document::AllocatorType &allocator = d.GetAllocator();

		for (auto &&item: block.entries) {
			Value r(kArrayType);
			for (auto &&ident: item.second) {
				Value v(kObjectType);
				ident.toJson(v, allocator);
				r.PushBack(v, allocator);
			}
			d.AddMember(StringRef(item.first.c_str()), r, allocator);
		}

		StringBuffer sb;
		PrettyWriter<StringBuffer> pw(sb);
		d.Accept(pw);

		string filename = this->mountpoint + "/" + BlockDevice::DIR_BLOCKS_DIR + "/" + to_string(id);
		ofstream file(filename);
		if (!file.is_open())
			return errno != 0 ? errno : ENOENT;
		file << sb.GetString() << endl;
		file.close();

		return this->SUCCESS;
	}

	int BlockDevice::addBlock(uint64_t *blockId) {
		unique_lock<mutex> lk(this->blockMutex);
		if (this->freeBlocks.size() == 0) {
			*blockId = this->nextFreeBlock;
			this->nextFreeBlock++;
		} else {
			*blockId = this->freeBlocks[this->freeBlocks.size() - 1];
			this->freeBlocks.pop_back();
		}
		lk.unlock();

		string filename = this->mountpoint + "/" + BLOCKS_DIR + "/" + to_string(*blockId);

		return createFile(filename);
	}

	int BlockDevice::delBlock(const uint64_t &blockId) {
		unique_lock<mutex> lk(this->blockMutex);
		if (this->nextFreeBlock - 1 == blockId)
			nextFreeBlock--;
		else
			this->freeBlocks.push_back(blockId);
		lk.unlock();

		string filename = this->mountpoint + "/" + BLOCKS_DIR + "/" + to_string(blockId);

		return deleteFile(filename);
	}

	int BlockDevice::getBlock(const uint64_t &blockId, std::uint8_t *buffer) {
		string filename = this->mountpoint + "/" + BLOCKS_DIR + "/" + to_string(blockId);
		ifstream file(filename);
		if (!file.is_open())
			return errno != 0 ? errno : ENOENT;

		file.read((char *) buffer, this->blockSize);
		file.close();
		return this->SUCCESS;
	}

	int BlockDevice::putBlock(const uint64_t &blockId, const uint8_t *buffer) {
		string filename = this->mountpoint + "/" + BLOCKS_DIR + "/" + to_string(blockId);
		ofstream file(filename);
		if (!file.is_open())
			return errno != 0 ? errno : ENOENT;

		file.write((const char *) buffer, this->blockSize);
		file.close();
		return this->SUCCESS;
	}

	bool BlockDevice::getBlockMetas(const uint64_t &blockId, mtfs::blockInfo_t &metas) {
		string filename = this->mountpoint + "/" + BLOCK_METAS_DIR + "/" + to_string(blockId) + ".json";
		ifstream file(filename);
		if (!file.is_open())
			return false;

		IStreamWrapper wrapper(file);

		Document d;
		d.ParseStream(wrapper);

		assert(d.HasMember(BI_REFF));
		assert(d[BI_REFF].HasMember(ID_POOL));
		assert(d[BI_REFF].HasMember(ID_VOLUME));
		assert(d[BI_REFF].HasMember(ID_ID));
		assert(d.HasMember(BI_ACCESS));

		metas.referenceId.poolId = d[BI_REFF][ID_POOL].GetUint();
		metas.referenceId.volumeId = d[BI_REFF][ID_VOLUME].GetUint();
		metas.referenceId.id = d[BI_REFF][ID_ID].GetUint64();
		metas.lastAccess = d[BI_ACCESS].GetUint64();

		return true;
	}

	bool BlockDevice::putBlockMetas(const uint64_t &blockId, const blockInfo_t &metas) {
		Document d;
		d.SetObject();
		Document::AllocatorType &allocator = d.GetAllocator();

		Value v;

		v.SetObject();
		v.AddMember(StringRef(ID_POOL), Value(metas.referenceId.poolId), allocator);
		v.AddMember(StringRef(ID_VOLUME), Value(metas.referenceId.volumeId), allocator);
		v.AddMember(StringRef(ID_ID), Value(metas.referenceId.id), allocator);
		d.AddMember(StringRef(BI_REFF), v, allocator);

		v.SetUint64(metas.lastAccess);
		d.AddMember(StringRef(BI_ACCESS), v, allocator);

		StringBuffer bStrBuff;
		PrettyWriter<StringBuffer> bWriter(bStrBuff);
		d.Accept(bWriter);

		string blockFilename = this->mountpoint + "/" + BLOCK_METAS_DIR + "/" + to_string(blockId) + ".json";
		ofstream blockFile;
		blockFile.open(blockFilename);
		blockFile << bStrBuff.GetString() << endl;
		blockFile.close();

		return true;
	}

	bool BlockDevice::getSuperblock(mtfs::superblock_t &superblock) {
		return false;
	}

	bool BlockDevice::putSuperblock(const superblock_t &superblock) {
		ofstream sbFile(this->mountpoint + "/" + METAS_DIR + "/superblock", ios::binary);
		sbFile.write((const char *) &superblock, sizeof(superblock));
		sbFile.close();

		return true;
	}

	/////////////Private method///////////////

	void BlockDevice::initDirHierarchie() {
		if (!dirExists(this->mountpoint + "/" + BlockDevice::INODES_DIR))
			mkdir((this->mountpoint + "/" + BlockDevice::INODES_DIR).c_str(), 0700);
		if (!dirExists(this->mountpoint + "/" + BlockDevice::DIR_BLOCKS_DIR))
			mkdir((this->mountpoint + "/" + BlockDevice::DIR_BLOCKS_DIR).c_str(), 0700);
		if (!dirExists(this->mountpoint + "/" + BlockDevice::BLOCKS_DIR))
			mkdir((this->mountpoint + "/" + BlockDevice::BLOCKS_DIR).c_str(), 0700);
		if (!dirExists(this->mountpoint + "/" + BlockDevice::METAS_DIR))
			mkdir((this->mountpoint + "/" + BlockDevice::METAS_DIR).c_str(), 0700);
		if (!dirExists(this->mountpoint + "/" + BlockDevice::BLOCK_METAS_DIR))
			mkdir((this->mountpoint + "/" + BlockDevice::BLOCK_METAS_DIR).c_str(), 0700);
	}

	void BlockDevice::initInodes() {
		string inodeFilename = this->mountpoint + "/" + METAS_DIR + "/inodes.json";
		ifstream inodeFile(inodeFilename);
		if (!inodeFile.is_open())
			return;

		IStreamWrapper isw(inodeFile);

		Document d;
		d.ParseStream(isw);

		assert(d.IsObject());
		assert(d.HasMember("nextFreeInode"));
		assert(d.HasMember("freeInodes"));
		this->nextFreeInode = d["nextFreeInode"].GetUint64();
		const Value &inodeArray = d["freeInodes"];

		this->freeInodes.clear();
		assert(inodeArray.IsArray());
		for (SizeType i = 0; i < inodeArray.Size(); i++) {
			this->freeInodes.push_back(inodeArray[i].GetUint64());
		}
		return;
	}

	void BlockDevice::initDirBlocks() {
		string filename = this->mountpoint + "/" + METAS_DIR + "/dirBlocks.json";
		ifstream inodeFile(filename);
		if (!inodeFile.is_open())
			return;

		IStreamWrapper isw(inodeFile);

		Document d;
		d.ParseStream(isw);

		assert(d.IsObject());
		assert(d.HasMember("nextFreeDirBlock"));
		assert(d.HasMember("freeDirBlocks"));
		this->nextFreeDirBlock = d["nextFreeDirBlock"].GetUint64();
		const Value &array = d["freeDirBlocks"];

		this->freeDirBlocks.clear();
		assert(array.IsArray());
		for (SizeType i = 0; i < array.Size(); i++) {
			this->freeDirBlocks.push_back(array[i].GetUint64());
		}
	}

	void BlockDevice::initBlocks() {
		string inodeFilename = this->mountpoint + "/" + METAS_DIR + "/blocks.json";
		ifstream inodeFile(inodeFilename);
		if (!inodeFile.is_open())
			return;

		IStreamWrapper isw(inodeFile);

		Document d;
		d.ParseStream(isw);

		assert(d.IsObject());
		assert(d.HasMember("nextFreeBlock"));
		assert(d.HasMember("freeBlocks"));
		this->nextFreeBlock = d["nextFreeBlock"].GetUint64();
		const Value &inodeArray = d["freeBlocks"];

		this->freeBlocks.clear();
		assert(inodeArray.IsArray());
		for (SizeType i = 0; i < inodeArray.Size(); i++) {
			this->freeBlocks.push_back(inodeArray[i].GetUint64());
		}
	}

	void BlockDevice::writeMetas() {
		{
//		writeInodeMeta
			Document d;
			d.SetObject();

			Document::AllocatorType &allocator = d.GetAllocator();

			Value freeInode(kObjectType);
			freeInode.SetUint64(this->nextFreeInode);
			d.AddMember(StringRef("nextFreeInode"), freeInode, allocator);

			Value inodeList(kArrayType);
			Value inode(kObjectType);

			for (auto b = this->freeInodes.begin(); b != this->freeInodes.end(); b++) {
				inode.SetUint64(*b);
				inodeList.PushBack(Value(*b), allocator);
			}

			d.AddMember(StringRef("freeInodes"), inodeList, allocator);

			StringBuffer strBuff;
			PrettyWriter<StringBuffer> writer(strBuff);
			d.Accept(writer);

			string inodeFilename = this->mountpoint + "/" + METAS_DIR + "/inodes.json";
			ofstream inodeFile;
			inodeFile.open(inodeFilename);
			inodeFile << strBuff.GetString() << endl;
			inodeFile.close();
		}

		{
//		writeDirBlocksMeta
			Document d;
			d.SetObject();

			Document::AllocatorType &allocator = d.GetAllocator();

			Value freeInode(kObjectType);
			freeInode.SetUint64(this->nextFreeDirBlock);
			d.AddMember(StringRef("nextFreeDirBlock"), freeInode, allocator);

			Value inodeList(kArrayType);
			Value inode(kObjectType);

			for (auto b = this->freeDirBlocks.begin(); b != this->freeDirBlocks.end(); b++) {
				inode.SetUint64(*b);
				inodeList.PushBack(Value(*b), allocator);
			}

			d.AddMember(StringRef("freeDirBlocks"), inodeList, allocator);

			StringBuffer strBuff;
			PrettyWriter<StringBuffer> writer(strBuff);
			d.Accept(writer);

			string inodeFilename = this->mountpoint + "/" + METAS_DIR + "/dirBlocks.json";
			ofstream inodeFile;
			inodeFile.open(inodeFilename);
			inodeFile << strBuff.GetString() << endl;
			inodeFile.close();
		}

		{
//		Write block metas
			Document db;
			db.SetObject();

			Document::AllocatorType &blAllocator = db.GetAllocator();

			Value freeBlock(kObjectType);
			freeBlock.SetUint64(this->nextFreeBlock);
			db.AddMember(StringRef("nextFreeBlock"), freeBlock, blAllocator);

			Value blockList(kArrayType);

			for (auto b :this->freeBlocks) {
				blockList.PushBack(Value(b), blAllocator);
			}

			db.AddMember(StringRef("freeBlocks"), blockList, blAllocator);

			StringBuffer bStrBuff;
			PrettyWriter<StringBuffer> bWriter(bStrBuff);
			db.Accept(bWriter);

			string blockFilename = this->mountpoint + "/" + METAS_DIR + "/blocks.json";
			ofstream blockFile;
			blockFile.open(blockFilename);
			blockFile << bStrBuff.GetString() << endl;
			blockFile.close();
		}
	}

	void BlockDevice::logError(string message) {
		cerr << message << endl;
	}

	bool BlockDevice::dirExists(string path) {
		struct stat info;

		if (stat(path.c_str(), &info) != 0)
			return false;
		else return (info.st_mode & S_IFDIR) != 0;
	}

	int BlockDevice::createFile(string path) {
		if (creat(path.c_str(), 0600) < 0) {
			string message = "create " + path + " error " + string(strerror(errno)) + " [" + to_string(errno) + "]";
			logError(message);
			return errno;
		}
		return this->SUCCESS;
	}

	int BlockDevice::deleteFile(std::string path) {
		if (remove(path.c_str()) != 0) {
			string message = "ERROR: deleting file " + path;
			logError(message);
			return errno;
		}
		return SUCCESS;
	}


}  // namespace Plugin

extern "C" pluginSystem::Plugin *createObj() {
	return new pluginSystem::BlockDevice();
}

extern "C" void destroyObj(pluginSystem::Plugin *plugin) {
	delete plugin;
}

extern "C" pluginSystem::pluginInfo_t getInfo() {
	vector<string> params;
	params.push_back("devicePath");
	params.push_back("fsType");

	pluginSystem::pluginInfo_t info = {
			.name = "block",
			.params = params,
	};

	return info;
}