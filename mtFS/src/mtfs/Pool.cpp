/**
 * \file Pool.cpp
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

#include <utils/Logger.h>
#include "mtfs/Pool.h"

using namespace std;

namespace mtfs {
	Pool::Pool(const size_t blkSize) : blockSize(blkSize) {}

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

		rule->configureStorage(volume);

		return 0;
	}

	int Pool::add(const ruleInfo_t &info, std::vector<ident_t> &idents, const blockType type, const int nb) {
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

				this->volumes[vid]->add(id, type);

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

				this->volumes[vid]->add(tmpIds, nbToAllocate, type);

				for (auto &&tmpId: tmpIds) {
					idents.push_back(ident_t(tmpId, vid));
				}
			}
		}

		return ret;
	}

	int Pool::del(const uint32_t &volumeId, const uint64_t &id, const blockType type) {
		return this->volumes[volumeId]->del(id, type);
	}

	int Pool::get(const uint32_t &volumeId, const uint64_t &id, void *data, const blockType type) {
		return this->volumes[volumeId]->get(id, data, type);
	}

	int Pool::put(const uint32_t &volumeId, const uint64_t &id, const void *data, const blockType type) {
		return this->volumes[volumeId]->put(id, data, type);
	}

	int Pool::getMetas(const uint32_t &volumeId, const uint64_t &id, blockInfo_t &metas, const blockType type) {
		return this->volumes[volumeId]->getMetas(id, metas, type);
	}

	int Pool::putMetas(const uint32_t &volumeId, const uint64_t &id, const blockInfo_t &metas, const blockType type) {
		return this->volumes[volumeId]->putMetas(id, metas, type);
	}

	int Pool::getValidVolumes(const ruleInfo_t &info, vector<uint32_t> &volumeIds) {
		for (auto &&item: this->rules) {
			if (item.second->satisfyRules(info))
				volumeIds.push_back(item.first);
		}
		return 0;
	}

	void
	Pool::doMigration(std::map<ident_t, ident_t> &movedBlk, std::vector<ident_t> &unsatisfyBlk, const blockType type) {
		for (auto &&volume :this->volumes) {
			Logger::getInstance()->log("Pool.doMigration", "do migration for volume: " + to_string(volume.first),
									   Logger::L_DEBUG);
			vector<blockInfo_t> unsatisfy;
			unsatisfy.clear();
			volume.second->getUnsatisfy(unsatisfy, type);

			for (auto &&blkInfos :unsatisfy) {
				ident_t oldIdent(blkInfos.id.id, volume.first);
				blkInfos.id.volumeId = volume.first;

				ruleInfo_t info = ruleInfo_t();
				info.lastAccess = blkInfos.lastAccess;

				vector<uint32_t> newVolumes;
				this->getValidVolumes(info, newVolumes);

				ident_t newIdent(0, newVolumes.front());
				this->volumes[newIdent.volumeId]->add(newIdent.id, type);

				if (oldIdent.volumeId == newIdent.volumeId) {
					continue;
				}

				void *datas = nullptr;
				switch (type) {
					case INODE:
						datas = new inode_t();
						break;
					case DIR_BLOCK:
						datas = new dirBlock_t();
						break;
					case DATA_BLOCK:
						datas = new uint8_t[this->blockSize];
						break;
					default:
						continue;
				}
				this->volumes[oldIdent.volumeId]->get(oldIdent.id, datas, type);

				this->volumes[newIdent.volumeId]->put(newIdent.id, datas, type);
				this->volumes[newIdent.volumeId]->putMetas(newIdent.id, blkInfos, type);

				movedBlk.emplace(oldIdent, newIdent);

//				this->volumes[oldIdent.volumeId]->del(oldIdent.id, type);

				switch (type) {
					case INODE:
						delete ((inode_t *) datas);
						break;
					case DIR_BLOCK:
						delete ((dirBlock_t *) datas);
						break;
					case DATA_BLOCK:
						delete[]((uint8_t *) datas);
						break;
					default:
						break;
				}
			}

			unsigned long nb = unsatisfy.size();

			if (0 != nb)
				Logger::getInstance()->log("Pool.doMigration", "Block need to move: " + to_string(nb), Logger::L_DEBUG);
		}
	}


}  // namespace mtfs
