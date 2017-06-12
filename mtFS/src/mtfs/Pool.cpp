#include "mtfs/Pool.h"

using namespace std;

namespace mtfs {

	Pool::~Pool() {
		for (auto &&vol: this->volumes) {
			delete vol.second;
		}
		for (auto &&rule: this->rules) {
			delete rule.second;
		}
	}

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

	void Pool::structToJson(const pool_t &pool, rapidjson::Value &dest, rapidjson::Document::AllocatorType &allocator) {
		dest.AddMember(rapidjson::StringRef(Rule::MIGRATION), rapidjson::Value(pool.migration), allocator);

		pool.rule->toJson(dest, allocator);

		rapidjson::Value volumes(rapidjson::kObjectType);
		for (auto &&item : pool.volumes) {
			rapidjson::Value volume(rapidjson::kObjectType);

			Volume::structToJson(item.second, volume, allocator);

			string id = to_string(item.first);
			rapidjson::Value index(id.c_str(), (rapidjson::SizeType) id.size(), allocator);
			volumes.AddMember(index, volume, allocator);
		}

		dest.AddMember(rapidjson::StringRef(Volume::VOLUMES), volumes, allocator);
	}

	void Pool::jsonToStruct(rapidjson::Value &src, pool_t &pool) {
		assert(src.HasMember(Rule::MIGRATION));
		pool.migration = src[Rule::MIGRATION].GetInt();

		assert(src.HasMember(Volume::VOLUMES));
		for (auto &&item :src[Volume::VOLUMES].GetObject()) {
			uint32_t id = (uint32_t) stoul(item.name.GetString());
			volume_t volume;
			memset(&volume, 0, sizeof(volume_t));
			volume.params.clear();

			volume.rule = Rule::buildRule(pool.migration, item.value);

			Volume::jsonToStruct(item.value, volume);

			pool.volumes.insert(make_pair(id, volume));
		}

	}

	int Pool::addVolume(uint32_t volumeId, Volume *volume, Rule *rule) {
		if (this->volumes.find(volumeId) != this->volumes.end() && this->rules.find(volumeId) != this->rules.end())
			return VOLUME_ID_EXIST;

		this->volumes[volumeId] = volume;
		this->rules[volumeId] = rule;

		return 0;
	}

	int Pool::addInode(const ruleInfo_t &info, vector<ident_t> &idents, const int nb) {
		return this->add(info, idents, nb, this->INODE);
	}

	bool Pool::delInode(uint16_t volumeId, uint64_t inodeId) {
		return false;
	}

	bool Pool::readInode(uint16_t volumeId, uint64_t inodeId) {
		return false;
	}

	int Pool::putInode(const uint32_t &volumeId, const uint64_t &inodeId, const inode_t &inode) {
		return this->volumes[volumeId]->putInode(inodeId, inode);
	}

	int Pool::addDirBlock(const ruleInfo_t &info, std::vector<ident_t> &idents, const int nb) {
		return this->add(info, idents, nb, this->DIR_BLOCK);
	}

	int Pool::delDirBlock(const uint32_t &volumeId, const uint64_t &id) {
		return 0;
	}

	int Pool::getDirBlock(const uint32_t &volumeId, const uint64_t &id, dirBlock_t &block) {
		return this->volumes[volumeId]->getDirBlock(id, block);
	}

	int Pool::putDirBlock(const uint32_t &volumeId, const uint64_t &id, const dirBlock_t &block) {
		return this->volumes[volumeId]->putDirBlock(id, block);
	}

	int Pool::addBlock(const ruleInfo_t &info, vector<ident_t> &idents, const int nb) {
		return this->add(info, idents, nb, this->BLOCK);
	}

	bool Pool::delBlock(uint16_t volumeId, uint64_t blockId) {
		return false;
	}

	bool Pool::readBlock(uint16_t volumeId, uint64_t blockId, uint8_t *block) {
		return false;
	}

	int Pool::putBlock(const uint32_t &volumeId, const uint64_t &blockId, const uint8_t *block) {
		return this->volumes[volumeId]->putBlock(blockId, block);
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


	int Pool::add(const ruleInfo_t &info, std::vector<ident_t> &idents, const int nb, const Pool::queryType &type) {
		int ret;
		vector<uint32_t> volumeIds;
		if (0 != (ret = this->getValidVolumes(info, volumeIds))) {
			return ret;
		}

		if (0 == volumeIds.size())
			return NO_VALID_VOLUME;

		if (nb <= volumeIds.size()) {
			for (int i = 0; i < nb; i++) {
				uint64_t id = 0;
				const uint32_t vid = volumeIds[i];

				switch (type) {
					case INODE:
						this->volumes[vid]->addInode(id);
						break;
					case DIR_BLOCK:
						this->volumes[vid]->addDirBlock(id);
						break;
					case BLOCK:
						this->volumes[vid]->addBlock(id);
						break;
				}

				idents.push_back(ident_t(id, vid));
			}
		} else {
			int blkPerPool = (int) (nb / volumeIds.size());
			int remainder = (int) (nb % volumeIds.size());
			for (auto &&vid: volumeIds) {
				vector<uint64_t> tmpIds;
				int nbToAllocate = blkPerPool;

				if (0 < remainder) {
					nbToAllocate++;
					remainder--;
				}

				switch (type) {
					case INODE:
						this->volumes[vid]->addInode(tmpIds, nbToAllocate);
						break;
					case DIR_BLOCK:
						this->volumes[vid]->addDirBlock(tmpIds, nbToAllocate);
						break;
					case BLOCK:
						this->volumes[vid]->addBlock(tmpIds, nbToAllocate);
						break;
				}

				for (auto &&tmpId: tmpIds) {
					idents.push_back(ident_t(tmpId, vid));
				}
			}
		}

		return ret;
	}

	int Pool::getValidVolumes(const ruleInfo_t &info, vector<uint32_t> &volumeIds) {
		for (auto &&item: this->rules) {
			if (item.second->satisfyRules(info))
				volumeIds.push_back(item.first);
		}
		return 0;
	}


}  // namespace mtfs
