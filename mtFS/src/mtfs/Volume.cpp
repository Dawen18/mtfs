#include <pluginSystem/PluginManager.h>
#include "mtfs/Volume.h"

using namespace std;

namespace mtfs {
	bool Volume::validate(const rapidjson::Value &volume) {
		if (!volume.HasMember(TYPE))
			return false;

		pluginSystem::PluginManager *manager = pluginSystem::PluginManager::getInstance();

		pluginSystem::Plugin *plugin = manager->getPlugin(volume[TYPE].GetString());
		if (plugin == NULL)
			return false;

		vector<string> infos = plugin->getInfos();
		infos.erase(infos.begin());
		for (auto i:infos)
			if (!volume.HasMember(i.c_str()))
				return false;

		return true;
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

	bool Volume::uptadeLastAcces(uint64_t id, std::map<uint64_t, std::vector<uint64_t>> map) {
		return false;
	}

	vector<string> Volume::getInfos() {
		return plugin->getInfos();
	}

	bool Volume::attach(std::map<std::string, std::string> params) {
		return plugin->attach(params);
	}

	bool Volume::detach() {
		return plugin->detach();
	}

	bool Volume::addInode(std::uint64_t &inodeId) {
		return plugin->addInode(inodeId);
	}

	bool Volume::delInode(std::uint64_t inodeId) {
		return plugin->delInode(inodeId);
	}

	bool Volume::readInode(std::uint64_t inodeId, mtfs::inode_st &inode) {
		return plugin->readInode(inodeId, inode);
	}

	bool Volume::writeInode(std::uint64_t inodeId, mtfs::inode_st &inode) {
		return plugin->writeInode(inodeId, inode);
	}

	bool Volume::addBlock(std::uint64_t &blockId) {
		return plugin->addBlock(blockId);
	}

	bool Volume::delBlock(std::uint64_t blockId) {
		return plugin->delBlock(blockId);
	}

	bool Volume::readBlock(std::uint64_t blockId, std::uint8_t *buffer) {
		return plugin->readBlock(blockId, buffer);
	}

	bool Volume::writeBlock(std::uint64_t blockId, std::uint8_t *buffer) {
		return plugin->writeBlock(blockId, buffer);
	}

	bool Volume::readSuperblock(mtfs::superblock_t &superblock) {
		return plugin->readSuperblock(superblock);
	}

	bool Volume::writeSuperblock(superblock_t &superblock) {
		return plugin->writeSuperblock(superblock);
	}
}  // namespace mtfs
