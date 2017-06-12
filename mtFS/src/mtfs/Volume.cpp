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


	int Volume::addInode(uint64_t &inodeId) {
		return this->plugin->addInode(&inodeId);
	}

	int Volume::addInode(std::vector<uint64_t> &ids, const int nb) {
		return this->add(ids, nb, this->INODE);
	}

	int Volume::delInode(const uint64_t &inodeId) {
		return plugin->delInode(inodeId);
	}

	int Volume::getInode(const uint64_t &inodeId, mtfs::inode_st &inode) {
		return plugin->getInode(inodeId, inode);
	}

	int Volume::putInode(const uint64_t &inodeId, const inode_t &inode) {
		return plugin->putInode(inodeId, inode);
	}

	int Volume::addDirBlock(uint64_t &blockId) {
		return this->plugin->addDirBlock(&blockId);
	}

	int Volume::addDirBlock(std::vector<uint64_t> &ids, const int nb) {
		return this->add(ids, nb, this->DIR_BLOCK);
	}

	int Volume::delDirBlock(const std::uint64_t &blockId) {
		return 0;
	}

	int Volume::getDirBlock(const std::uint64_t &blockId, dirBlock_t &block) {
		return this->plugin->getDirBlock(blockId, block);
	}

	int Volume::putDirBlock(const std::uint64_t &blockId, const dirBlock_t &block) {
		return this->plugin->putDirBlock(blockId, block);
	}

	int Volume::addBlock(std::uint64_t &blockId) {
		return plugin->addBlock(&blockId);
	}

	int Volume::addBlock(std::vector<uint64_t> &ids, const int nb) {
		return this->add(ids, nb, this->BLOCK);
	}

	bool Volume::delBlock(std::uint64_t blockId) {
		return plugin->delBlock(blockId);
	}

	bool Volume::getBlock(std::uint64_t blockId, std::uint8_t *buffer) {
		return plugin->getBlock(blockId, buffer);
	}

	int Volume::putBlock(const uint64_t blockId, const uint8_t *buffer) {
		return plugin->putBlock(blockId, buffer);
	}

	bool Volume::getSuperblock(mtfs::superblock_t &superblock) {
		return plugin->getSuperblock(superblock);
	}

	bool Volume::putSuperblock(superblock_t &superblock) {
		return plugin->putSuperblock(superblock);
	}


	int Volume::add(std::vector<uint64_t> &ids, const int nb, const Volume::queryType &type) {
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
				case BLOCK:
					thPool.schedule(bind(&pluginSystem::Plugin::addBlock, this->plugin, id));
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

	bool Volume::uptadeLastAcces(uint64_t id, std::map<uint64_t, std::vector<uint64_t>> map) {
		return false;
	}


}  // namespace mtfs
