#include <pluginSystem/PluginManager.h>
#include <pluginSystem/Plugin.h>
#include <thread>
#include <boost/thread.hpp>
#include <boost/threadpool/pool.hpp>
#include "mtfs/Volume.h"

using namespace std;

namespace mtfs {
	Volume::~Volume() {
		plugin->detach();

		pluginSystem::PluginManager::getInstance()->freePlugin(this->plugin->getName(), this->plugin);
	}

	bool Volume::validate(const rapidjson::Value &volume) {
		if (!volume.HasMember(pluginSystem::Plugin::TYPE))
			return false;

		if (!volume.HasMember(pluginSystem::Plugin::PARAMS))
			return false;

		string volType = volume[pluginSystem::Plugin::TYPE].GetString();

		pluginSystem::PluginManager *manager = pluginSystem::PluginManager::getInstance();

		pluginSystem::Plugin *plugin = manager->getPlugin(volType);
		if (plugin == NULL)
			return false;

		pluginSystem::pluginInfo_t info;

		if (manager->getInfo(volType, info) != pluginSystem::PluginManager::SUCCESS)
			return false;

		for (auto &&inf : info.params) {
			if (!volume[pluginSystem::Plugin::PARAMS].HasMember(inf.c_str()))
				return false;
		}

//		vector<string> infos = plugin->getInfos();
//		infos.erase(infos.begin());
//		for (auto i:infos)
//			if (!volume.HasMember(i.c_str()))
//				return false;

		return true;
	}

	void Volume::structToJson(const volume_t &volume, rapidjson::Value &dest,
							  rapidjson::Document::AllocatorType &allocator) {
		volume.rule->toJson(dest, allocator);

		rapidjson::Value v;

		v.SetString(rapidjson::StringRef(volume.pluginName.c_str()));
		dest.AddMember(rapidjson::StringRef(pluginSystem::Plugin::TYPE), v, allocator);

		rapidjson::Value p(rapidjson::kObjectType);
		for (auto &&item : volume.params) {
			v.SetString(rapidjson::StringRef(item.second.c_str()));
			p.AddMember(rapidjson::StringRef(item.first.c_str()), v, allocator);
		}
		dest.AddMember(rapidjson::StringRef(pluginSystem::Plugin::PARAMS), p, allocator);
	}

	void Volume::jsonToStruct(rapidjson::Value &src, volume_t &volume) {
		assert(src.HasMember(pluginSystem::Plugin::TYPE));
		volume.pluginName = src[pluginSystem::Plugin::TYPE].GetString();

		assert(src.HasMember(pluginSystem::Plugin::PARAMS));
		for (auto &&item : src[pluginSystem::Plugin::PARAMS].GetObject()) {
			volume.params.insert(make_pair(item.name.GetString(), item.value.GetString()));
		}
	}

	Volume::Volume(pluginSystem::Plugin *plugin) : plugin(plugin) {}

	bool Volume::setTimeLimits(int low, int high) {
		return false;
	}

	void Volume::getBlockInfo(uint64_t blockId, blockInfo_t &info) {
	}

	void Volume::setBlockInfo(uint64_t blockId, blockInfo_t &info) {
	}

	std::vector<ident_t> Volume::getBlocksBelowLimit() {
		vector<ident_t> blocks;

		return blocks;
	}

	std::vector<ident_t> Volume::getBlocksAboveLimit() {
		vector<ident_t> blocks;

		return blocks;
	}

	std::vector<ident_t> Volume::getInodesBelowLimit() {
		vector<ident_t> blocks;

		return blocks;
	}

	std::vector<ident_t> Volume::getInodesAboveLimit() {
		vector<ident_t> blocks;

		return blocks;
	}

	bool Volume::getSuperblock(mtfs::superblock_t &superblock) {
		return plugin->getSuperblock(superblock);
	}

	bool Volume::putSuperblock(superblock_t &superblock) {
		return plugin->putSuperblock(superblock);
	}

	int Volume::add(uint64_t &id, const Acces::queryType type) {
		int ret;
		switch (type) {
			case Acces::INODE:
				ret = this->plugin->addInode(&id);
				break;
			case Acces::DIR_BLOCK:
				ret = this->plugin->addDirBlock(&id);
				break;
			case Acces::DATA_BLOCK:
				ret = this->plugin->addBlock(&id);
				break;
			default:
				ret = ENOSYS;
//				TODO log noimplemented
				break;
		}
		return ret;
	}

	int Volume::add(std::vector<uint64_t> &ids, const int nb, const Acces::queryType type) {
		boost::threadpool::pool thPool((size_t) nb);

		vector<uint64_t *> tmp;
		for (int i = 0; i < nb; ++i) {
			uint64_t *id = new uint64_t();
			tmp.push_back(id);
			switch (type) {
				case Acces::INODE:
					thPool.schedule(bind(&pluginSystem::Plugin::addInode, this->plugin, id));
					break;
				case Acces::DIR_BLOCK:
					thPool.schedule(bind(&pluginSystem::Plugin::addDirBlock, this->plugin, id));
					break;
				case Acces::DATA_BLOCK:
					thPool.schedule(bind(&pluginSystem::Plugin::addBlock, this->plugin, id));
					break;
				default:
//					TODO log
					break;
			}
		}

		thPool.wait();

		for (auto &&item: tmp) {
			ids.push_back(*item);
			delete item;
		}

		return 0;
	}

	int Volume::del(const uint64_t &id, const Acces::queryType type) {
		int ret;

		switch (type) {
			case Acces::INODE:
				ret = this->plugin->delInode(id);
				break;
			case Acces::DIR_BLOCK:
				ret = this->plugin->delDirBlock(id);
				break;
			case Acces::DATA_BLOCK:
				ret = this->plugin->delBlock(id);
				break;
			default:
				ret = ENOSYS;
//				TODO log noimplemented
				break;
		}

		return ret;
	}

	int Volume::get(const uint64_t &id, void *data, Acces::queryType type) {
		int ret;

		switch (type) {
			case Acces::INODE:
				ret = this->plugin->getInode(id, *(inode_t *) data);
				break;
			case Acces::DIR_BLOCK:
				ret = this->plugin->getDirBlock(id, *(dirBlock_t *) data);
				break;
			case Acces::DATA_BLOCK:
				ret = this->plugin->getBlock(id, (uint8_t *) data);
				break;
			default:
				ret = ENOSYS;
//				TODO log noimplemented
				break;
		}

		return ret;
	}

	int Volume::put(const uint64_t &id, const void *data, Acces::queryType type) {
		int ret;

		switch (type) {
			case Acces::INODE:
				ret = this->plugin->putInode(id, *(inode_t *) data);
				break;
			case Acces::DIR_BLOCK:
				ret = this->plugin->putDirBlock(id, *(dirBlock_t *) data);
				break;
			case Acces::DATA_BLOCK:
				ret = this->plugin->putBlock(id, (uint8_t *) data);
				break;
			default:
				ret = ENOSYS;
//				TODO log noimplemented
				break;
		}

		return ret;
	}

	bool Volume::uptadeLastAcces(uint64_t id, std::map<uint64_t, std::vector<uint64_t>> map) {
		return false;
	}


}  // namespace mtfs
