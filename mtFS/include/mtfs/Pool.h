#ifndef FILESTORAGE_POOL_H
#define FILESTORAGE_POOL_H

#include <string>
#include <vector>
#include <mtfs/structs.h>
#include <mtfs/Volume.h>
#include <mtfs/Rule.h>
#include <pluginSystem/Plugin.h>

namespace mtfs {
	class Pool {
	public:
		static constexpr const char *POOLS = "pools";

//		STATUS CODE
		static const int SUCCESS = 0;
		static const int VOLUME_ID_EXIST = 1;
		static const int NO_VALID_VOLUME = 2;


	private:
		enum queryType {
			INODE,
			DIR_BLOCK,
			BLOCK,
		};

		std::map<uint32_t, Volume *> volumes;

		std::map<uint32_t, Rule *> rules;

		int blockSize;


	public:
		virtual ~Pool();

		static bool validate(const rapidjson::Value &pool);

		static void
		structToJson(const pool_t &pool, rapidjson::Value &dest, rapidjson::Document::AllocatorType &allocator);

		static void jsonToStruct(rapidjson::Value &src, pool_t &pool);

		int addVolume(uint32_t volumeId, Volume *volume, Rule *rule);

		int addInode(const ruleInfo_t &info, std::vector<ident_t> &idents, const int nb = 1);

		bool delInode(uint16_t volumeId, uint64_t inodeId);

		int getInode(const uint32_t &volumeId, const uint64_t &inodeId, inode_t &inode);

		int putInode(const uint32_t &volumeId, const uint64_t &inodeId, const inode_t &inode);

		int addDirBlock(const ruleInfo_t &info, std::vector<ident_t> &idents, const int nb = 1);

		int delDirBlock(const uint32_t &volumeId, const uint64_t &id);

		int getDirBlock(const uint32_t &volumeId, const uint64_t &id, dirBlock_t &block);

		int putDirBlock(const uint32_t &volumeId, const uint64_t &id, const dirBlock_t &block);

		int addBlock(const ruleInfo_t &info, std::vector<ident_t> &idents, const int nb = 1);

		bool delBlock(uint16_t volumeId, uint64_t blockId);

		bool readBlock(uint16_t volumeId, uint64_t blockId, uint8_t *block);

		int putBlock(const uint32_t &volumeId, const uint64_t &blockId, const uint8_t *block);


		std::vector<superblock_t> readSuperblocks();

		bool writeSuperblocks(superblock_t baseSuperblock);

		void setBlockInfo(ident_t blockId, blockInfo_t &info);

		void getBlockInfo(ident_t blockId, blockInfo_t &info);

		void moveBlocks(std::vector<move_t> &asMoved, std::vector<ident_t> &needOtherPool);


	private:
		int add(const ruleInfo_t &info, std::vector<ident_t> &idents, const int nb, const queryType &type);

		int getValidVolumes(const ruleInfo_t &info, std::vector<uint32_t> &volumeIds);
	};

}  // namespace mtfs
#endif
