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
	} ident_t;

	struct inode_st {
	public:
		std::uint16_t accesRight;

		std::uint16_t uid;

		std::uint16_t gid;

		std::uint64_t size;

		std::uint8_t linkCount;

		std::vector<std::uint64_t> access;

		std::vector<ident_st> referenceId;

		std::vector<ident_st> dataBlocks;

	};
	struct superblock_t {
	};

	typedef struct block_st {
		ident_t id;
		ident_t reversePointer;
		std::vector<uint64_t> lastAccess;
	} block_t;
}  // namespace mtFS

#endif //TRAVAIL_BACHELOR_STRUCTS_H
