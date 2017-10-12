/**
 * \file Pool.h
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

#ifndef FILESTORAGE_POOL_H
#define FILESTORAGE_POOL_H

#include <string>
#include <vector>
#include <mtfs/structs.h>
//#include <mtfs/Volume.h>
#include <mtfs/Rule.h>

namespace mtfs {
	class Volume;

	class Pool {

	public:
		static constexpr const char *POOLS = "pools";

//		STATUS CODE
		static const int SUCCESS = 0;
		static const int VOLUME_ID_EXIST = 1;
		static const int NO_VALID_VOLUME = 2;


	public:
		Pool(const size_t &blkSize);

		virtual ~Pool();

		static bool validate(const rapidjson::Value &pool);

		static void
		structToJson(const pool_t &pool, rapidjson::Value &dest, rapidjson::Document::AllocatorType &allocator);

		static void jsonToStruct(rapidjson::Value &src, pool_t &pool);

		int addVolume(uint32_t volumeId, Volume *volume, Rule *rule);

		int add(const ruleInfo_t &info, std::vector<ident_t> &idents, const blockType &type, const int &nb = 1);

		int del(const uint32_t &volumeId, const uint64_t &id, const blockType &type);

		int get(const uint32_t &volumeId, const uint64_t &id, void *data, const blockType &type);

		int put(const uint32_t &volumeId, const uint64_t &id, const void *data, const blockType &type);

		int getMetas(const uint32_t &volumeId, const uint64_t &id, blockInfo_t &metas, const blockType &type);

		int putMetas(const uint32_t &volumeId, const uint64_t &id, const blockInfo_t &metas, const blockType &type);

		void
		doMigration(std::map<ident_t, ident_t> &movedBlk, std::vector<ident_t> &unsatisfyBlk, const blockType &type);


	private:
		const size_t blockSize;

		std::map<uint32_t, Volume *> volumes;
		std::map<uint32_t, Rule *> rules;


		int getValidVolumes(const ruleInfo_t &info, std::vector<uint32_t> &volumeIds);
	};

}  // namespace mtfs
#endif
