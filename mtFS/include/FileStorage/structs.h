/**
 * @author David Wittwer
 * @date 22.04.17.
**/

#ifndef TRAVAIL_BACHELOR_STRUCTS_H
#define TRAVAIL_BACHELOR_STRUCTS_H

namespace FileStorage {
	struct id_st {

	};

	struct inode_st {
	public:
		std::uint16_t accesRight;

		std::uint16_t uid;

		std::uint16_t gid;

		std::uint64_t size;

		std::vector <std::uint16_t> access;

		std::uint8_t linkCount;

		std::vector <id_st> dataBlocks;

	};
	struct superblock_t {
	};
}  // namespace mtFS

#endif //TRAVAIL_BACHELOR_STRUCTS_H
