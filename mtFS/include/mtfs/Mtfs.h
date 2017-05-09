#ifndef FILESTORAGE_MTFS_H
#define FILESTORAGE_MTFS_H

#include <thread>

#include <mtfs/Rule.h>
#include <mtfs/InodeAcces.h>
#include <mtfs/BlockAccess.h>
#include <mtfs/DirectoryEntryAccess.h>
#include <rapidjson/document.h>

namespace mtfs {
	class Mtfs {
	public:
		static constexpr const char *INODE_CACHE = "inodeCacheSize";
		static constexpr const char *DIR_CACHE = "directoryCacheSize";
		static constexpr const char *BLOCK_CACHE = "blockCacheSize";
		static constexpr const char *BLOCK_SIZE = "blockSize";
		static constexpr const char *REDUNDANCY = "redundancy";

	private:
		InodeAcces *inodes;
		BlockAccess *blocs;
		DirectoryEntryAccess *dirEntry;
		static Mtfs *instance;
		static std::thread *thr;


	public:
		static Mtfs *getInstance();

		static bool validate(const rapidjson::Value &system);

		static void start();

		static void join();

		bool build(const rapidjson::Value &system, std::string homeDir);


	private:
		Mtfs();

		static void loop();
	};

}  // namespace mtfs
#endif
