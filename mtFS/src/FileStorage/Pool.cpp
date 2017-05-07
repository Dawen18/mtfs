#include "mtfs/Pool.h"

using namespace std;

namespace mtfs {

	int Pool::addVolume(uint32_t volumeId, Volume *volume, Rule *rule) {
		if (this->volumes.find(volumeId) == this->volumes.end() && this->rules.find(volumeId) == this->rules.end())
			return VOLUME_ID_EXIST;

		this->volumes[volumeId] = volume;
		this->rules[volumeId] = rule;

		return 0;
	}

	bool Pool::addBlock(ruleInfo_t &info, uint16_t &volumeId, uint64_t &blockId) {
		return false;
	}

	bool Pool::delBlock(uint16_t volumeId, uint64_t blockId) {
		return false;
	}

	bool Pool::readBlock(uint16_t volumeId, uint64_t blockId, uint8_t *block) {
		return false;
	}

	bool Pool::writeBlock(uint16_t volumeId, uint64_t blockId, uint8_t *block) {
		return false;
	}

	bool Pool::addInode(ruleInfo_t &info, uint16_t &volumeId, uint64_t &inodeId) {
		return false;
	}

	bool Pool::delInode(uint16_t volumeId, uint64_t inodeId) {
		return false;
	}

	bool Pool::readInode(uint16_t volumeId, uint64_t inodeId) {
		return false;
	}

	bool Pool::writeInode(uint16_t volumeId, uint64_t inodeId) {
		return false;
	}

	vector<superblock_t> Pool::readSuperblocks() {
		vector<superblock_t> superblocks;

		return superblocks;
	}

	bool Pool::writeSuperblocks(superblock_t baseSuperblock) {
		return false;
	}

	void Pool::setBlockInfo(ident_t blockId, blockInfo_t &info) {
	}

	void Pool::getBlockInfo(ident_t blockId, blockInfo_t &info) {
	}

	void Pool::moveBlocks(vector<move_t> &asMoved, vector<ident_t> &needOtherPool) {
	}

	bool Pool::chooseVolume(ruleInfo_t &info, PluginSystem::Plugin *volume) {
		return false;
	}
}  // namespace mtfs
