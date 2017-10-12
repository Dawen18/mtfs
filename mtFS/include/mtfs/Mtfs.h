/**
 * \file Mtfs.h
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

#ifndef FILESTORAGE_MTFS_H
#define FILESTORAGE_MTFS_H

#include <thread>

#include <mtfs/Rule.h>
#include <mtfs/Acces.h>
#include <rapidjson/document.h>
#include <boost/threadpool.hpp>
#include <fuse3/fuse_lowlevel.h>
#include "structs.h"
#include "Migrator.h"
#include <mutex>
#include <condition_variable>
#include <utils/Semaphore.h>

namespace mtfs {
	struct internalInode_st;
	struct dl_st;

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
//		CONFIG
		static const size_t SIMULT_DL = 2;
		static const size_t SIMULT_UP = 2;
		static const int INIT_DL = 2;
		static constexpr const double ATTR_TIMEOUT = 1.0;

//		REQUEST STATUS CODES
		static const int SUCCESS = 0;
		static const int PENDING = 9999;

		static Mtfs *instance;
		static boost::threadpool::pool *threadPool;
		static std::string systemName;

		size_t redundancy;
		size_t blockSize;
		int maxEntryPerBlock;
		internalInode_st *rootIn;

		Acces *inodes;
		Acces *dirBlocks;
		Acces *blocks;

		std::thread migratorThr;
		Migrator::info_st migratorInfo;

	public:
		static Mtfs *getInstance();

		/**
		 * Validate system config
		 *
		 * @param system JSON config of system
		 *
		 * @return true if validelse false
		 */
		static bool validate(const rapidjson::Value &system);

		/**
		 * Create the root inode
		 *
		 * @param [out] inode The root inode
		 *
		 * @return true if success
		 */
		static bool createRootInode(inode_t &inode);

		/**
		 * Start MTFS
		 *
		 * @param system System config
		 * @param homeDir MTFS home dir
		 * @param sysName System name
		 *
		 * @return true if success
		 */
		static bool start(rapidjson::Document &system, std::string homeDir, std::string sysName);

		/**
		 * Stop MTFS
		 */
		static void stop();

		/**
		 * Convert struct superblock to JSON document
		 *
		 * @param [in] sb Superblock struct
		 * @param [out] d JSON document
		 */
		static void structToJson(const superblock_t &sb, rapidjson::Document &d);

		/**
		 * Convert JSON document to struct superblock
		 *
		 * @param [in] d JSON document
		 * @param [out] sb Struct superblock
		 */
		static void jsonToStruct(rapidjson::Document &d, superblock_t &sb);

		/*						Fuse handlers						*/

		/**
		 * Init handler
		 *
		 * @param userdata
		 * @param conn
		 */
		void init(void *userdata, fuse_conn_info *conn);

		/**
		 * Destroy handler
		 *
		 * @param userdata
		 */
		void destroy(void *userdata);

		/**
		 * Lookup handler
		 *
		 * @param req
		 * @param parent
		 * @param name
		 */
		void lookup(fuse_req_t req, fuse_ino_t parent, const std::string &name);

		/**
		 * Getattr handler
		 *
		 * @param req
		 * @param ino
		 * @param fi
		 */
		void getAttr(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi);

		/**
		 * Setattr handler
		 *
		 * @param req
		 * @param ino
		 * @param attr
		 * @param toSet
		 * @param fi
		 */
		void setAttr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int toSet, fuse_file_info *fi);

		/**
		 * Mknod handler
		 *
		 * @param req
		 * @param parent
		 * @param name
		 * @param mode
		 * @param rdev
		 */
		void mknod(fuse_req_t req, fuse_ino_t parent, const std::string &name, mode_t mode, dev_t rdev);

		/**
		 * Mkdir handler
		 *
		 * @param req
		 * @param ino
		 * @param name
		 * @param mode
		 */
		void mkdir(fuse_req_t req, fuse_ino_t ino, const std::string &name, mode_t mode);

		/**
		 * Unlink handler
		 *
		 * @param req
		 * @param parent
		 * @param name
		 */
		void unlink(fuse_req_t req, fuse_ino_t parent, const std::string &name);

		/**
		 * Rmdir handler
		 *
		 * @param req
		 * @param parent
		 * @param name
		 */
		void rmdir(fuse_req_t req, fuse_ino_t parent, const std::string &name);

		/**
		 * Open handler
		 *
		 * @param req
		 * @param ino
		 * @param fi
		 */
		void open(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi);

		/**
		 * Read handler
		 *
		 * @param req
		 * @param ino
		 * @param size
		 * @param off
		 * @param fi
		 */
		void read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi);

		/**
		 * Write handler
		 *
		 * @param req
		 * @param ino
		 * @param buf
		 * @param size
		 * @param off
		 * @param fi
		 */
		void write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, fuse_file_info *fi);

		/**
		 * Release handler
		 *
		 * @param req
		 * @param ino
		 * @param fi
		 */
		void release(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi);

		/**
		 * Opendir handler
		 *
		 * @param req
		 * @param ino
		 * @param fi
		 */
		void opendir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi);

		/**
		 * Readdir handler
		 *
		 * @param req
		 * @param ino
		 * @param size
		 * @param off
		 * @param fi
		 */
		void readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi);

		/**
		 * Releasedir handler
		 *
		 * @param req
		 * @param ino
		 * @param fi
		 */
		void releasedir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi);

		/**
		 * Access handler
		 *
		 * @param req
		 * @param ino
		 * @param mask
		 */
		void access(fuse_req_t req, fuse_ino_t ino, int mask);

		/**
		 * Write_buf handler
		 *
		 * @param req
		 * @param ino
		 * @param bufv
		 * @param off
		 * @param fi
		 */
		void write_buf(fuse_req_t req, fuse_ino_t ino, fuse_bufvec *bufv, off_t off, fuse_file_info *fi);

		/**
		 * Readdirplus handler
		 *
		 * @param req
		 * @param ino
		 * @param size
		 * @param off
		 * @param fi
		 */
		void readdirplus(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi);


	private:

		Mtfs();


		inode_t getRootInode();

		bool build(const superblock_t &superblock);

		void readRootInode();

		void writeRootInode();

		int addEntry(internalInode_st *parentInode, std::string name, std::vector<ident_t> &inodeIds);

		int insertInode(const inode_t &inode, std::vector<ident_t> &idents);

		void dlBlocks(const inode_t &inode, dl_st *dlSt, const blockType type, const int firstBlockIdx);

		void dlDirBlocks(std::vector<ident_t> &ids, std::queue<dirBlock_t> *q, std::mutex *queueMutex, Semaphore *sem);

		void dlInodes(dl_st *src, dl_st *dst);

		void initMetas(const internalInode_st &parentInode, const std::vector<ident_t> &ids, const blockType &type,
					   boost::threadpool::pool *thPool = nullptr);

		void
		dlInode(std::vector<ident_t> &ids, std::queue<std::pair<std::string, inode_t>> *queue, std::mutex *queueMutex,
				Semaphore *sem,
				std::string &key);


		////							UTILS							////

		internalInode_st *getIntInode(fuse_ino_t ino);

		void
		doReaddir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi, const bool &plus = false);

		size_t dirBufAdd(fuse_req_t &req, char *buf, size_t &currentSize, std::string name, internalInode_st &inode,
						 const bool &plus);

		void buildParam(const internalInode_st &inode, fuse_entry_param &param);

		void buildStat(const internalInode_st &inode, struct stat &st);

		ruleInfo_t getRuleInfo(const inode_t &inode);

		static internalInode_st *newInode(const mode_t &mode, const fuse_ctx *ctx);

		static uint64_t now();

		int doUnlink(internalInode_st *parent, const std::string name);

		int delEntry(std::vector<ident_t> &ids, dirBlock_t &blk, const std::string &name);
	};

}  // namespace mtfs
#endif
