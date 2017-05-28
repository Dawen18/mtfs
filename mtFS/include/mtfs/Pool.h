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
		static const int VOLUME_ID_EXIST = -1;


	private:
		std::map<uint32_t, Volume *> volumes;

		std::map<uint32_t, Rule *> rules;

		int blockSize;


	public:
		static bool validate(const rapidjson::Value &pool);

		static void
		structToJson(const pool_t &pool, rapidjson::Value &dest, rapidjson::Document::AllocatorType &allocator);

		static void jsonToStruct(rapidjson::Value &src, pool_t &pool);

		int addVolume(uint32_t volumeId, Volume *volume, Rule *rule);

		bool addBlock(ruleInfo_t &info, uint16_t &volumeId, uint64_t &blockId);

		bool delBlock(uint16_t volumeId, uint64_t blockId);

		bool readBlock(uint16_t volumeId, uint64_t blockId, uint8_t *block);

		bool writeBlock(uint16_t volumeId, uint64_t blockId, uint8_t *block);

		bool addInode(ruleInfo_t &info, uint16_t &volumeId, uint64_t &inodeId);

		bool delInode(uint16_t volumeId, uint64_t inodeId);

		bool readInode(uint16_t volumeId, uint64_t inodeId);

		bool writeInode(uint16_t volumeId, uint64_t inodeId);

		std::vector<superblock_t> readSuperblocks();

		bool writeSuperblocks(superblock_t baseSuperblock);

		void setBlockInfo(ident_t blockId, blockInfo_t &info);

		void getBlockInfo(ident_t blockId, blockInfo_t &info);

		void moveBlocks(std::vector<move_t> &asMoved, std::vector<ident_t> &needOtherPool);


	private:
		bool chooseVolume(ruleInfo_t &info, pluginSystem::Plugin *volume);
	};

}  // namespace mtfs
#endif
