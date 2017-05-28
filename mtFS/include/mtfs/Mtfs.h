#ifndef FILESTORAGE_MTFS_H
#define FILESTORAGE_MTFS_H

#include <thread>

#include <mtfs/Rule.h>
#include <mtfs/InodeAcces.h>
#include <mtfs/BlockAccess.h>
#include <mtfs/DirectoryEntryAccess.h>
#include <rapidjson/document.h>
#include <boost/threadpool.hpp>
#include <fuse3/fuse_lowlevel.h>
#include "structs.h"

namespace mtfs {
	class Mtfs {
	public:
		static constexpr const char *SYSTEMS_DIR = "Systems";
		static constexpr const char *CONFIG_DIR = "Configs";

		static constexpr const char *INODE_CACHE = "inodeCacheSize";
		static constexpr const char *DIR_CACHE = "directoryCacheSize";
		static constexpr const char *BLOCK_CACHE = "blockCacheSize";
		static constexpr const char *BLOCK_SIZE_ST = "blockSize";
		static constexpr const char *REDUNDANCY = "redundancy";

	private:
		static Mtfs *instance;
		static boost::threadpool::pool *threadPool;
		static std::string systemName;

		InodeAcces *inodes;
		BlockAccess *blocks;
		DirectoryEntryAccess *dirEntries;


	public:

		static bool validate(const rapidjson::Value &system);

		static bool createRootInode(inode_t &inode);

		static bool start(const rapidjson::Value &system, std::string homeDir, std::string sysName);

		static void stop();

		static void structToJson(const superblock_t &sb, rapidjson::Document &d);

		static void jsonToStruct(rapidjson::Document &d, superblock_t &sb);

		/*						Fuse fcts						*/

		static void getAttr(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi);

	private:
		static Mtfs *getInstance();

		Mtfs();

		bool build(const rapidjson::Value &system, std::string homeDir);

		void stat(fuse_req_t req, fuse_ino_t ino);

		int rootStat(struct stat &st);
	};

}  // namespace mtfs
#endif
