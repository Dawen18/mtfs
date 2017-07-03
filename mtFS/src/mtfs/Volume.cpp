#include <pluginSystem/PluginManager.h>
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

	void Volume::getBlockInfo(uint64_t blockId, blockInfo_t &info) {
	}

	void Volume::setBlockInfo(uint64_t blockId, blockInfo_t &info) {
	}

	int Volume::add(uint64_t &id, const queryType type) {
		int ret;
		switch (type) {
			case INODE:
				ret = this->plugin->addInode(&id);
				break;
			case DIR_BLOCK:
				ret = this->plugin->addDirBlock(&id);
				break;
			case DATA_BLOCK:
				ret = this->plugin->addBlock(&id);
				break;
			default:
				ret = ENOSYS;
//				TODO log noimplemented
				break;
		}
		return ret;
	}

	int Volume::add(std::vector<uint64_t> &ids, const int nb, const queryType type) {
		boost::threadpool::pool thPool((size_t) nb);

		vector<uint64_t *> tmp;
		for (int i = 0; i < nb; ++i) {
			uint64_t *id = new uint64_t();
			tmp.push_back(id);
			switch (type) {
				case INODE:
					thPool.schedule(bind(&pluginSystem::Plugin::addInode, this->plugin, id));
					break;
				case DIR_BLOCK:
					thPool.schedule(bind(&pluginSystem::Plugin::addDirBlock, this->plugin, id));
					break;
				case DATA_BLOCK:
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

	int Volume::del(const uint64_t &id, const queryType type) {
		int ret;

		switch (type) {
			case INODE:
				ret = this->plugin->delInode(id);
				break;
			case DIR_BLOCK:
				ret = this->plugin->delDirBlock(id);
				break;
			case DATA_BLOCK:
				ret = this->plugin->delBlock(id);
				break;
			default:
				ret = ENOSYS;
//				TODO log noimplemented
				break;
		}

		return ret;
	}

	int Volume::get(const uint64_t &id, void *data, queryType type) {
		int ret;

		switch (type) {
			case INODE:
				ret = this->plugin->getInode(id, *(inode_t *) data);
				break;
			case DIR_BLOCK:
				ret = this->plugin->getDirBlock(id, *(dirBlock_t *) data);
				break;
			case DATA_BLOCK:
				ret = this->plugin->getBlock(id, (uint8_t *) data);
				break;
			default:
				ret = ENOSYS;
//				TODO log noimplemented
				break;
		}

		this->updateLastAccess(id, type);

		return ret;
	}

	int Volume::put(const uint64_t &id, const void *data, queryType type) {
		int ret;

		switch (type) {
			case INODE:
				ret = this->plugin->putInode(id, *(inode_t *) data);
				break;
			case DIR_BLOCK:
				ret = this->plugin->putDirBlock(id, *(dirBlock_t *) data);
				break;
			case DATA_BLOCK:
				ret = this->plugin->putBlock(id, (uint8_t *) data);
				break;
			default:
				ret = ENOSYS;
//				TODO log noimplemented
				break;
		}

		return ret;
	}

	void Volume::accept(Visitor *v) {
		v->visit(this);
	}

	bool Volume::updateLastAccess(const uint64_t &id, const queryType type) {
		return false;
	}


}  // namespace mtfs
