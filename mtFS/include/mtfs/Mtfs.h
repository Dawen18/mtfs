#ifndef FILESTORAGE_MTFS_H
#define FILESTORAGE_MTFS_H

#include <thread>

#include <mtfs/Rule.h>
#include <mtfs/InodeAcces.h>
#include <mtfs/BlockAccess.h>
#include <mtfs/DirectoryBlockAccess.h>
#include <rapidjson/document.h>
#include <boost/threadpool.hpp>
#include <fuse3/fuse_lowlevel.h>
#include "structs.h"
#include <mutex>
#include <condition_variable>
#include <utils/Semaphore.h>

namespace mtfs {
	struct internalInode_st;

	class Mtfs {
	public:
//		static constexpr const char *SYSTEMS_DIR = "Systems";
		static constexpr const char *CONFIG_DIR = "Configs";

		static constexpr const char *INODE_CACHE = "inodeCacheSize";
		static constexpr const char *DIR_CACHE = "directoryCacheSize";
		static constexpr const char *BLOCK_CACHE = "blockCacheSize";
		static constexpr const char *BLOCK_SIZE_ST = "blockSize";
		static constexpr const char *REDUNDANCY = "redundancy";
		static constexpr const char *ROOT_INODES = "rootInodes";

	private:
//		REQUEST STATUS CODES
		static const int SUCCESS = 0;
		static const int PENDING = 9999;


		static Mtfs *instance;
		static boost::threadpool::pool *threadPool;
		static std::string systemName;

		int redundancy;
		int blockSize;
		int maxEntryPerBlock;
		internalInode_st *rootIn;

		InodeAcces *inodes;
		DirectoryBlockAccess *dirBlocks;
		BlockAccess *blocks;


	public:
		static Mtfs *getInstance();

		static bool validate(const rapidjson::Value &system);

		static bool createRootInode(inode_t &inode);

		static bool start(rapidjson::Document &system, std::string homeDir, std::string sysName);

		static void stop();

		static void structToJson(const superblock_t &sb, rapidjson::Document &d);

		static void jsonToStruct(rapidjson::Document &d, superblock_t &sb);

		/*						Fuse fcts						*/

		void init(void *userdata, fuse_conn_info *conn);

		void destroy(void *userdata);

		void getAttr(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi);

		void lookup(fuse_req_t req, fuse_ino_t parent, const std::string name);

		void mknod(fuse_req_t req, fuse_ino_t parent, const std::string name, mode_t mode, dev_t rdev);

		void access(fuse_req_t req, fuse_ino_t ino, int mask);

		void opendir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi);

	private:

		Mtfs();

		inode_t getRootInode();

		bool build(const superblock_t &superblock);

		void readRootInode();

		void writeRootInode();

		void stat(fuse_req_t req, fuse_ino_t ino);

		int rootStat(struct stat &st);

		int addEntry(internalInode_st *parentInode, std::string name, std::vector<ident_t> &inodeIds);

		int insertInode(const inode_t &inode, std::vector<ident_t> &idents);

		int insertDirBlock();

		int insertBlock();

		ruleInfo_t getRuleInfo(const inode_t &inode);

		/////////////////////////////////////
//		UTILS
		///////////////////////////////////////////

		static inode_t *newInode(const mode_t &mode, const fuse_ctx *ctx);

		static uint64_t now();
	};

}  // namespace mtfs
#endif
