#ifndef FILESTORAGE_MTFS_H
#define FILESTORAGE_MTFS_H

#include <thread>

#include <mtfs/Rule.h>
#include <mtfs/InodeAcces.h>
#include <mtfs/BlockAccess.h>
#include <mtfs/DirectoryEntryAccess.h>
#include <rapidjson/document.h>
#include <mutex>
#include <utils/ThreadQueue.h>

namespace mtfs {
	class Mtfs {
	public:
		static constexpr const char *CONFIG_DIR = "Configs";
		static constexpr const char *INODE_CACHE = "inodeCacheSize";
		static constexpr const char *DIR_CACHE = "directoryCacheSize";
		static constexpr const char *BLOCK_CACHE = "blockCacheSize";
		static constexpr const char *BLOCK_SIZE = "blockSize";
		static constexpr const char *REDUNDANCY = "redundancy";

	private:
		static Mtfs *instance;
		static std::thread *thr;
		static std::mutex mutex;
		static bool keepRunning;

		InodeAcces *inodes;
		BlockAccess *blocks;
		DirectoryEntryAccess *dirEntries;
		ThreadQueue<std::string> *synchronousQueue;


	public:
		static Mtfs *getInstance();

		static bool validate(const rapidjson::Value &system);

		static bool createRootInode(rapidjson::Document &d);

		void setSynchronousQueue(ThreadQueue<std::string> *synchronousQueue);

		void start();

		void join();

		bool build(const rapidjson::Value &system, std::string homeDir);

		void processSynchronous();


	private:
		Mtfs();

		static void loop();
	};

}  // namespace mtfs
#endif
