/**
 * @author David Wittwer
 * @date 22.04.17.
**/

#ifndef TRAVAIL_BACHELOR_STRUCTS_H
#define TRAVAIL_BACHELOR_STRUCTS_H

namespace FileStorage {
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
	} ident_t;

	typedef struct inode_st {
		uint16_t accesRight;
		uint16_t uid;
		uint16_t gid;
		uint64_t size;
		uint8_t linkCount;
		std::vector<std::uint64_t> access;
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

			if (referenceId.size() != rhs.referenceId.size())
				return false;
			for (auto lb = referenceId.begin(), rb = rhs.referenceId.begin();
				 lb != referenceId.end(); lb++, rb++) {
				ident_t li = *lb, ri = *rb;

				if (li != ri)
					return false;
			}

			if (dataBlocks.size() != rhs.dataBlocks.size()) {
				std::cout << "size differ " << dataBlocks.size() << " " << rhs.dataBlocks.size() << std::endl;
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
	} inode_t;

	typedef struct superblock_st {
	} superblock_t;

	typedef struct block_st {
		ident_t id;
		ident_t reversePointer;
		std::vector<uint64_t> lastAccess;
	} block_t;
}  // namespace mtFS

#endif //TRAVAIL_BACHELOR_STRUCTS_H
