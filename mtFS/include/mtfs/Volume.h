/**
 * \file Volume.h
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

#ifndef FILESTORAGE_VOLUME_H
#define FILESTORAGE_VOLUME_H

#include <string>
#include <vector>
#include <climits>

#include <rapidjson/document.h>
#include <mtfs/structs.h>
#include <pluginSystem/Plugin.h>
#include <mutex>

namespace mtfs {
	class Volume {

	public:
		static constexpr const char *VOLUMES = "volumes";

	private:
		static const uint64_t NEW_DELAY = 50;

		pluginSystem::Plugin *plugin;

		bool isTimeVolume;
		uint64_t minDelay;
		uint64_t maxDelay;

		std::mutex iaMutex;
		std::map<uint64_t, uint64_t> inodesAccess;
		std::mutex daMutex;
		std::map<uint64_t, uint64_t> dirBlockAccess;
		std::mutex baMutex;
		std::map<uint64_t, uint64_t> blocksAccess;

		std::mutex niMutex;
		std::map<uint64_t, uint64_t> newInode;
		std::mutex ndMutex;
		std::map<uint64_t, uint64_t> newDir;
		std::mutex nbMutex;
		std::map<uint64_t, uint64_t> newData;

	public:
		virtual ~Volume();

		void setMinDelay(uint64_t minDelay);

		void setMaxDelay(uint64_t maxDelay);

		void setIsTimeVolume(bool b);

		static bool validate(const rapidjson::Value &volume);

		static void
		structToJson(const volume_t &volume, rapidjson::Value &dest, rapidjson::Document::AllocatorType &allocator);

		static void jsonToStruct(rapidjson::Value &src, volume_t &volume);

		explicit Volume(pluginSystem::Plugin *plugin);

		int add(uint64_t &id, const blockType &type);

		int add(std::vector<uint64_t> &ids, const int &nb, const blockType &type);

		int del(const uint64_t &id, const blockType &type);

		int get(const uint64_t &id, void *data, const blockType &type);

		int put(const uint64_t &id, const void *data, const blockType &type);

		int getMetas(const uint64_t &id, blockInfo_t &metas, const blockType &type);

		int putMetas(const uint64_t &id, const blockInfo_t &metas, const blockType &type);

		int getUnsatisfy(std::vector<blockInfo_t> &unsatisfy, const blockType &type, int limit = INT_MAX);

	private:
		int getOutOfTime(std::vector<uint64_t> &blocks, const blockType &type);

		bool updateLastAccess(const uint64_t &id, const blockType &type);

		void purgeNewMap(const blockType &type);
	};

}  // namespace mtfs
#endif
