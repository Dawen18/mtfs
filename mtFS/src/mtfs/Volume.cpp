/**
 * \file Volume.cpp
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

#include <pluginSystem/PluginManager.h>
#include <thread>
#include <boost/thread.hpp>
#include <boost/threadpool/pool.hpp>
#include "mtfs/Volume.h"
#include <mtfs/Rule.h>

using namespace std;

namespace mtfs {

	Volume::Volume(pluginSystem::Plugin *plugin) : plugin(plugin), minDelay(0), maxDelay(0) {}

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
		if (nullptr == plugin)
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

	int Volume::add(uint64_t &id, const blockType &type) {
		int ret;
		ret = this->plugin->add(&id, type);

		if (ENOSYS == ret) {
//				TODO log noimplemented
		}
		mutex *mu = nullptr;
		map<uint64_t, uint64_t> *ma = nullptr;

		switch (type) {
			case INODE:
				mu = &this->niMutex;
				ma = &this->newInode;
				break;
			case DIR_BLOCK:
				mu = &this->ndMutex;
				ma = &this->newData;
				break;
			case DATA_BLOCK:
				mu = &this->nbMutex;
				ma = &this->newData;
				break;
			default:
				return ENOSYS;
		}

		//			add id in newBlock map
		unique_lock<mutex> lk(*mu);
		ma->emplace(id, time(nullptr));
		lk.unlock();

		return ret;
	}

	int Volume::add(std::vector<uint64_t> &ids, const int &nb, const blockType &type) {
		boost::threadpool::pool thPool((size_t) nb);

		vector<uint64_t *> tmp;
		for (int i = 0; i < nb; ++i) {
			uint64_t *id;
			id = new uint64_t();
			tmp.push_back(id);
			thPool.schedule(bind(&pluginSystem::Plugin::add, this->plugin, id, type));
		}

		thPool.wait();

		mutex *mu = nullptr;
		map<uint64_t, uint64_t> *ma = nullptr;

		switch (type) {
			case INODE:
				mu = &this->niMutex;
				ma = &this->newInode;
				break;
			case DIR_BLOCK:
				mu = &this->ndMutex;
				ma = &this->newData;
				break;
			case DATA_BLOCK:
				mu = &this->nbMutex;
				ma = &this->newData;
				break;
			default:
				return ENOSYS;
		}

		for (auto &&item: tmp) {
//			add id in newBlock map
			unique_lock<mutex> lk(*mu);
			ma->emplace(*item, time(nullptr));
			lk.unlock();

			ids.push_back(*item);
			delete item;
		}

		return 0;
	}

	int Volume::del(const uint64_t &id, const blockType &type) {
		int ret;
		ret = this->plugin->del(id, type);

		if (ENOSYS == ret) {
//				TODO log noimplemented
		}

		map<uint64_t, uint64_t> *access = nullptr;
		mutex *mu;
		switch (type) {
			case INODE:
				mu = &this->iaMutex;
				access = &this->inodesAccess;
				break;
			case DIR_BLOCK:
				mu = &this->daMutex;
				access = &this->dirBlockAccess;
				break;
			case DATA_BLOCK:
				mu = &this->baMutex;
				access = &this->blocksAccess;
				break;
			default:
				return ENOSYS;
		}

		unique_lock<mutex> lk(*mu);
		access->erase(id);
		lk.unlock();

		return ret;
	}

	int Volume::get(const uint64_t &id, void *data, const blockType &type) {
		int ret;
		ret = this->plugin->get(id, data, type, false);

		if (ENOSYS == ret) {
//				TODO log noimplemented
		}

		this->updateLastAccess(id, type);

		return ret;
	}

	int Volume::put(const uint64_t &id, const void *data, const blockType &type) {
		int ret;
		ret = this->plugin->put(id, data, type, false);

		if (ENOSYS == ret) {
//				TODO log noimplemented
		}

		this->updateLastAccess(id, type);

		return ret;
	}

	int Volume::getMetas(const uint64_t &id, blockInfo_t &metas, const blockType &type) {
		int ret;
		if (EXIT_SUCCESS != (ret = this->plugin->get(id, &metas, type, true))) {
			return ret;
		}

		map<uint64_t, uint64_t> *access = nullptr;
		mutex *mu;
		switch (type) {
			case INODE:
				mu = &this->iaMutex;
				access = &this->inodesAccess;
				break;
			case DIR_BLOCK:
				mu = &this->daMutex;
				access = &this->dirBlockAccess;
				break;
			case DATA_BLOCK:
				mu = &this->baMutex;
				access = &this->blocksAccess;
				break;
			default:
				return ENOSYS;
		}

		unique_lock<mutex> lk(*mu);
		if (access->end() == access->find(id))
			(*access)[id] = metas.lastAccess;

		return ret;
	}

	int Volume::putMetas(const uint64_t &id, const blockInfo_t &metas, const blockType &type) {
		if (0 == id && INODE == type)
			return EXIT_SUCCESS;

		int ret;
		if (EXIT_SUCCESS != (ret = this->plugin->put(id, &metas, type, true))) {
			return ret;
		}

		map<uint64_t, uint64_t> *access = nullptr;
		mutex *mu;
		switch (type) {
			case INODE:
				mu = &this->iaMutex;
				access = &this->inodesAccess;
				break;
			case DIR_BLOCK:
				mu = &this->daMutex;
				access = &this->dirBlockAccess;
				break;
			case DATA_BLOCK:
				mu = &this->baMutex;
				access = &this->blocksAccess;
				break;
			default:
				return ENOSYS;
		}

		unique_lock<mutex> lk(*mu);
		(*access)[id] = metas.lastAccess;

		return ret;
	}

	bool Volume::updateLastAccess(const uint64_t &id, const blockType &type) {
		blockInfo_t metas = blockInfo_t();
		this->getMetas(id, metas, type);

		metas.lastAccess = static_cast<uint64_t>(time(nullptr));
		this->putMetas(id, metas, type);

		return true;
	}

	void Volume::setMinDelay(uint64_t minDelay) {
		this->minDelay = minDelay;
	}

	void Volume::setMaxDelay(uint64_t maxDelay) {
		this->maxDelay = maxDelay;
	}

	void Volume::setIsTimeVolume(bool b) {
		this->isTimeVolume = b;
	}

	int Volume::getUnsatisfy(vector<blockInfo_t> &unsatisfy, const blockType &type, const int limit) {
		this->purgeNewMap(type);

		int nb = 0;
		if (this->isTimeVolume) {
			vector<uint64_t> under;
			this->getOutOfTime(under, type);

			for (auto &&blk :under) {
				blockInfo_t info = blockInfo_t();
				this->getMetas(blk, info, type);
				info.id.id = blk;

				unsatisfy.push_back(info);

				nb++;
				if (limit == nb)
					return 0;
			}
		}

		return 0;
	}

	int Volume::getOutOfTime(vector<uint64_t> &blocks, const blockType &type) {
		uint64_t now;
		now = (uint64_t) time(nullptr);
		const uint64_t maxTimestamp = now - this->minDelay;
		const uint64_t minTimestamp = 0 == this->maxDelay ? 0 : now - this->maxDelay;

		mutex *mu;
		map<uint64_t, uint64_t> *mp;
		mutex *newMut;
		map<uint64_t, uint64_t> *newMap;
		switch (type) {
			case INODE:
				mu = &this->iaMutex;
				mp = &this->inodesAccess;
				newMut = &this->niMutex;
				newMap = &this->newInode;
				break;
			case DIR_BLOCK:
				mu = &this->daMutex;
				mp = &this->dirBlockAccess;
				newMut = &this->ndMutex;
				newMap = &this->newDir;
				break;
			case DATA_BLOCK:
				mu = &this->baMutex;
				mp = &this->blocksAccess;
				newMut = &this->ndMutex;
				newMap = &this->newData;
				break;
			default:
				return ENOSYS;
		}

		unique_lock<mutex> lk(*mu);
		for (auto &&item :*mp) {
			if (0 == item.first && INODE == type)
				continue;
			unique_lock<mutex> nlk(*newMut);
			if (newMap->end() != newMap->find(item.first)) {
				nlk.unlock();
				continue;
			}
			nlk.unlock();

			if (!(maxTimestamp > item.second && minTimestamp < item.second))
				blocks.push_back(item.first);
		}

		return 0;
	}

	void Volume::purgeNewMap(const blockType &type) {
		mutex *mu = nullptr;
		map<uint64_t, uint64_t> *ma = nullptr;

		switch (type) {
			case INODE:
				mu = &this->niMutex;
				ma = &this->newInode;
				break;
			case DIR_BLOCK:
				mu = &this->ndMutex;
				ma = &this->newData;
				break;
			case DATA_BLOCK:
				mu = &this->nbMutex;
				ma = &this->newData;
				break;
			default:
				return;
		}

		uint64_t newTimestamp = time(nullptr) - NEW_DELAY;

		unique_lock<mutex> lk(*mu);
		for (auto &&item :*ma) {
			if (newTimestamp > item.second)
				ma->erase(item.first);
		}
	}

}  // namespace mtfs
