/**
 * @author David Wittwer
 * @date 22.04.17.
**/

#ifndef TRAVAIL_BACHELOR_STRUCTS_H
#define TRAVAIL_BACHELOR_STRUCTS_H

#include <map>
#include <mtfs/Rule.h>
#include <rapidjson/document.h>

#define IN_MODE "accessRight"
#define IN_UID "uid"
#define IN_GID "gid"
#define IN_SIZE "size"
#define IN_LINKS "linkCount"
#define IN_ACCESS "access"
#define IN_REFF "referenceId"
#define IN_BLOCKS "dataBlocks"

#define ID_POOL "poolId"
#define ID_VOLUME "volumeId"
#define ID_ID "id"

#define RU_MIGRATION "migration"

#define PO_POOLS "pools"

#define SB_INODE_CACHE "inodeCacheSize"
#define SB_DIR_CACHE "directoryCacheSize"
#define SB_BLOCK_CACHE "blockCacheSize"
#define SB_BLOCK_SIZE_ST "blockSize"
#define SB_REDUNDANCY "redundancy"

namespace mtfs {

	class Rule;

	typedef struct ident_st {
		std::uint32_t poolId;
		std::uint32_t volumeId;
		std::uint64_t id;

		bool operator==(const ident_st &i) const {
			return (poolId == i.poolId && volumeId == i.volumeId && id == i.id);
		}

		bool operator!=(const ident_st &i) const {
			return (poolId != i.poolId && volumeId != i.volumeId && id != i.id);
		}

		void toJson(rapidjson::Value &dest, rapidjson::Document::AllocatorType &alloc) const {
			rapidjson::Value v;

			v.SetUint(this->poolId);
			dest.AddMember(rapidjson::StringRef(ID_POOL), v, alloc);

			v.SetUint(this->volumeId);
			dest.AddMember(rapidjson::StringRef(ID_VOLUME), v, alloc);

			v.SetUint64(this->id);
			dest.AddMember(rapidjson::StringRef(ID_ID), v, alloc);
		}
	} ident_t;

	typedef struct inode_st {
		uint16_t accesRight;
		uid_t uid;
		gid_t gid;
		uint64_t size;
		uint8_t linkCount;
		std::uint64_t access;
		std::vector<ident_t> referenceId;
		std::vector<std::vector<ident_t>> dataBlocks;

		bool operator==(const inode_st &rhs) const {
			if (accesRight != rhs.accesRight)
				return false;
			if (uid != rhs.uid)
				return false;
			if (gid != rhs.gid)
				return false;
			if (size != rhs.size)
				return false;
			if (linkCount != rhs.linkCount)
				return false;
			if (access != rhs.access)
				return false;

			if (referenceId.size() != rhs.referenceId.size())
				return false;
			for (auto lb = referenceId.begin(), rb = rhs.referenceId.begin();
				 lb != referenceId.end(); lb++, rb++) {
				ident_t li = *lb, ri = *rb;

				if (li != ri)
					return false;
			}

			if (dataBlocks.size() != rhs.dataBlocks.size()) {
//				std::cout << "size differ " << dataBlocks.size() << " " << rhs.dataBlocks.size() << std::endl;
				return false;
			}
			for (auto lIter = dataBlocks.begin(), rIter = rhs.dataBlocks.begin();
				 lIter != dataBlocks.end(); lIter++, rIter++) {
				std::vector<ident_t> red = *lIter, rhsRed = *rIter;

				if (red.size() != rhsRed.size())
					return false;
				for (auto lb = red.begin(), rb = rhsRed.begin();
					 lb != red.end(); lb++, rb++) {
					ident_t li = *lb, ri = *rb;

					if (li != ri)
						return false;
				}
			}

			return true;
		}

		void toJson(rapidjson::Document &dest) const {
			dest.SetObject();

			rapidjson::Value v;
			rapidjson::Document::AllocatorType &alloc = dest.GetAllocator();

			v.SetInt(this->accesRight);
			dest.AddMember(rapidjson::StringRef(IN_MODE), v, alloc);

			v.SetInt(this->uid);
			dest.AddMember(rapidjson::StringRef(IN_UID), v, alloc);

			v.SetInt(this->gid);
			dest.AddMember(rapidjson::StringRef(IN_GID), v, alloc);

			v.SetUint64(this->size);
			dest.AddMember(rapidjson::StringRef(IN_SIZE), v, alloc);

			v.SetUint(this->linkCount);
			dest.AddMember(rapidjson::StringRef(IN_LINKS), v, alloc);

			v.SetUint64(this->access);
			dest.AddMember(rapidjson::StringRef(IN_ACCESS), v, alloc);

			rapidjson::Value a(rapidjson::kArrayType);
			for (auto &&id : referenceId) {
				rapidjson::Value ident(rapidjson::kObjectType);

				id.toJson(ident, alloc);

				a.PushBack(ident, alloc);
			}
			dest.AddMember(rapidjson::StringRef(IN_REFF), a, alloc);

			a.SetArray();
			for (auto &&blocks : dataBlocks) {
				rapidjson::Value red(rapidjson::kArrayType);

				for (auto &&block : blocks) {
					rapidjson::Value ident(rapidjson::kObjectType);

					block.toJson(ident, alloc);

					red.PushBack(ident, alloc);
				}

				a.PushBack(red, alloc);
			}
			dest.AddMember(rapidjson::StringRef(IN_BLOCKS), a, alloc);
		}
	} inode_t;

	typedef struct volume_st {
		std::string pluginName;
		Rule *rule;
		std::map<std::string, std::string> params;
	} volume_t;

	typedef struct pool_st {
		int migration;
		Rule *rule;
		std::map<uint32_t, volume_t> volumes;
	} pool_t;

	typedef struct superblock_st {
		int iCacheSz;
		int dCacheSz;
		int bCacheSz;
		int blockSz;
		int redundancy;
		int migration;
		std::map<uint32_t, pool_t> pools;

//		void toJson(rapidjson::Document &d) const {
//			d.SetObject();
//			rapidjson::Document::AllocatorType &allocator = d.GetAllocator();
//
//			rapidjson::Value v;
//
//			v.SetInt(this->iCacheSz);
//			d.AddMember(rapidjson::StringRef(SB_INODE_CACHE), v, allocator);
//
//			v.SetInt(this->dCacheSz);
//			d.AddMember(rapidjson::StringRef(SB_DIR_CACHE), v, allocator);
//
//			v.SetInt(this->bCacheSz);
//			d.AddMember(rapidjson::StringRef(SB_BLOCK_CACHE), v, allocator);
//
//			v.SetInt(this->blockSz);
//			d.AddMember(rapidjson::StringRef(SB_BLOCK_SIZE_ST), v, allocator);
//
//			v.SetInt(this->redundancy);
//			d.AddMember(rapidjson::StringRef(SB_REDUNDANCY), v, allocator);
//
//			v.SetInt(this->migration);
//			d.AddMember(rapidjson::StringRef(RU_MIGRATION), v, allocator);
//
//			rapidjson::Value pools(rapidjson::kObjectType);
//			for (auto &&item : this->pools) {
//				rapidjson::Value pool(rapidjson::kObjectType);
//
////				Pool::structToJson(item.second, pool, allocator);
//
//				std::string id = std::to_string(item.first);
//				rapidjson::Value index(id.c_str(), (rapidjson::SizeType) id.size(), allocator);
//				pools.AddMember(index, pool, allocator);
//			}
//			d.AddMember(rapidjson::StringRef(PO_POOLS), pools, allocator);
//		}
	} superblock_t;

	typedef struct blockInfo_st {
		ident_st referenceId;
		std::vector<uint64_t> lastAccess;
	} blockInfo_t;

	typedef struct move_st {
		ident_t previousId;
		ident_t newId;
	} move_t;

}  // namespace mtFS

#endif //TRAVAIL_BACHELOR_STRUCTS_H
