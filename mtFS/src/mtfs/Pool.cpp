#include "mtfs/Pool.h"

using namespace std;

namespace mtfs {

	bool Pool::validate(const rapidjson::Value &pool) {
		if (!pool.HasMember(mtfs::Volume::VOLUMES))
			throw invalid_argument("Volumes missing!");
		if (!pool[mtfs::Volume::VOLUMES].IsObject())
			throw invalid_argument("Volumes is not a object");

		int migration = -1;
		if (pool[mtfs::Volume::VOLUMES].MemberCount() <= 0)
			throw invalid_argument("Number of volumes invalid!");
		else if (pool[mtfs::Volume::VOLUMES].MemberCount() != 1) {
			if (!pool.HasMember(Rule::MIGRATION))
				throw invalid_argument("Migration missing!");

			migration = pool[Rule::MIGRATION].GetInt();
		}

		for (auto &m: pool[mtfs::Volume::VOLUMES].GetObject()) {
			if (Rule::rulesAreValid(migration, m.value) != Rule::VALID_RULES)
				throw invalid_argument(string("Rules invalid for volume '") + m.name.GetString() + "'!");

			if (!Volume::validate(m.value))
				throw invalid_argument(string("Volume '") + m.name.GetString() + "' invalid!");
		}

		return true;
	}

	int Pool::addVolume(uint32_t volumeId, Volume *volume, Rule *rule) {
		if (this->volumes.find(volumeId) != this->volumes.end() && this->rules.find(volumeId) != this->rules.end())
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

	bool Pool::chooseVolume(ruleInfo_t &info, pluginSystem::Plugin *volume) {
		return false;
	}
}  // namespace mtfs
