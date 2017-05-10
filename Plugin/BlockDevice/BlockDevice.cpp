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

#include "BlockDevice.h"


using namespace std;
using namespace rapidjson;
using namespace mtfs;

namespace pluginSystem {
	BlockDevice::BlockDevice() {
		srand((unsigned int) time(NULL));
		this->nextFreeInode = 1;
	}

	vector<string> BlockDevice::getInfos() {
		vector<string> infos;
		infos.push_back("block");
		infos.push_back("devicePath");
		infos.push_back("fsType");

		return infos;
	}

	bool BlockDevice::attach(std::map<string, string> params) {
		if (params.find("devicePath") == params.end() || params.find("home") == params.end() ||
			params.find("fsType") == params.end())
			return false;

		string home;
		home = params.at("home");
		if (*home.end() != '/')
			home += '/';
		string parentDir = home + "BlockDevices/";
		this->fsType = params.at("fsType");
		this->devicePath = params.at("devicePath");
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

	bool BlockDevice::addInode(std::uint64_t &inodeId) {
		if (this->freeInodes.size() == 0) {
			inodeId = this->nextFreeInode;
			this->nextFreeInode++;
		} else {
			inodeId = this->freeInodes[0];
			this->freeInodes.erase(freeInodes.begin());
		}

		string filename = this->mountpoint + "/" + INODES_DIR + "/" + to_string(inodeId);

		return createFile(filename);
	}

	bool BlockDevice::delInode(std::uint64_t inodeId) {
		if (this->nextFreeInode - 1 == inodeId)
			this->nextFreeInode--;
		else {
			this->freeInodes.push_back(inodeId);
		}

		string filename = this->mountpoint + "/" + INODES_DIR + "/" + to_string(inodeId);

		return deleteFile(filename);
	}

	bool BlockDevice::readInode(std::uint64_t inodeId, mtfs::inode_st &inode) {
		string filename = this->mountpoint + "/" + INODES_DIR + "/" + to_string(inodeId);
		ifstream file(filename);
		if (!file.is_open())
			return false;

		IStreamWrapper wrapper(file);

		Document d;
		d.ParseStream(wrapper);

		assert(d.IsObject());
		assert(d.HasMember("accessRight"));
		assert(d.HasMember("uid"));
		assert(d.HasMember("gid"));
		assert(d.HasMember("size"));
		assert(d.HasMember("linkCount"));
		assert(d.HasMember("referenceId"));
		assert(d.HasMember("dataBlocks"));

		inode.accesRight = (uint16_t) d["accessRight"].GetUint();
		inode.uid = (uint16_t) d["uid"].GetUint();
		inode.gid = (uint16_t) d["gid"].GetUint();
		inode.size = d["size"].GetUint64();
		inode.linkCount = (uint8_t) d["linkCount"].GetUint();

		const Value &referenceArray = d["referenceId"];
		assert(referenceArray.IsArray());
		inode.referenceId.clear();
		for (auto &v : referenceArray.GetArray()) {
			ident_t ident;

			assert(v.IsObject());
			assert(v.HasMember("poolId"));
			assert(v.HasMember("volumeId"));
			assert(v.HasMember("id"));

			ident.poolId = (uint16_t) v["poolId"].GetUint();
			ident.volumeId = (uint16_t) v["volumeId"].GetUint();
			ident.id = v["id"].GetUint64();

			inode.referenceId.push_back(ident);
		}

		const Value &dataArray = d["dataBlocks"];
		assert(dataArray.IsArray());
		inode.dataBlocks.clear();
		for (auto &a : dataArray.GetArray()) {
			vector<ident_t> redundancy;

			assert(a.IsArray());
			for (auto &v : a.GetArray()) {
				ident_t ident;

				assert(v.IsObject());
				assert(v.HasMember("poolId"));
				assert(v.HasMember("volumeId"));
				assert(v.HasMember("id"));

				ident.poolId = (uint16_t) v["poolId"].GetUint();
				ident.volumeId = (uint16_t) v["volumeId"].GetUint();
				ident.id = v["id"].GetUint64();

				redundancy.push_back(ident);
			}

			inode.dataBlocks.push_back(redundancy);
		}

		return true;
	}

	bool BlockDevice::writeInode(std::uint64_t inodeId, mtfs::inode_st &inode) {
		Document d;
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


		StringBuffer sb;
		PrettyWriter<StringBuffer> pw(sb);
		d.Accept(pw);

		string filename = this->mountpoint + "/" + BlockDevice::INODES_DIR + "/" + to_string(inodeId);
		ofstream inodeFile(filename);
		inodeFile << sb.GetString() << endl;
		inodeFile.close();

		return true;
	}

	bool BlockDevice::addBlock(std::uint64_t &blockId) {
		return false;
	}

	bool BlockDevice::delBlock(std::uint64_t blockId) {
		(void) blockId;
		return false;
	}

	bool BlockDevice::readBlock(std::uint64_t blockId, std::uint8_t *buffer) {
		return false;
	}

	bool BlockDevice::writeBlock(std::uint64_t blockId, std::uint8_t *buffe) {
		return false;
	}

	bool BlockDevice::readSuperblock(mtfs::superblock_t &superblock) {
		return false;
	}

	bool BlockDevice::writeSuperblock(superblock_t &superblock) {
		return false;
	}

	/////////////Private method///////////////

	void BlockDevice::initDirHierarchie() {
		if (!dirExists(this->mountpoint + "/" + BlockDevice::INODES_DIR))
			mkdir((this->mountpoint + "/" + BlockDevice::INODES_DIR).c_str(), 0700);
		if (!dirExists(this->mountpoint + "/" + BlockDevice::BLOCKS_DIR))
			mkdir((this->mountpoint + "/" + BlockDevice::BLOCKS_DIR).c_str(), 0700);
		if (!dirExists(this->mountpoint + "/" + BlockDevice::METAS_DIR))
			mkdir((this->mountpoint + "/" + BlockDevice::METAS_DIR).c_str(), 0700);
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

	void BlockDevice::writeMetas() {

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

	void BlockDevice::logError(string message) {
		cerr << message << endl;
	}

	bool BlockDevice::dirExists(string path) {
		struct stat info;

		if (stat(path.c_str(), &info) != 0)
			return false;
		else return (info.st_mode & S_IFDIR) != 0;
	}

	bool BlockDevice::createFile(string path) {
		if (creat(path.c_str(), 0600) < 0) {
			string message = "create " + path + " error " + string(strerror(errno)) + " [" + to_string(errno) + "]";
			logError(message);
			return false;
		}
		return true;
	}

	bool BlockDevice::deleteFile(std::string path) {
		if (remove(path.c_str()) != 0) {
			string message = "ERROR: deleting file " + path;
			logError(message);
			return false;
		}
		return true;
	}


}  // namespace Plugin
