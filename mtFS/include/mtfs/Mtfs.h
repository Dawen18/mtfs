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
		static const int TIME_MIGRATION = 0;
		static const int RIGHT_MIGRATION = 1;

	private:
		InodeAcces *inodes;
		BlockAccess *blocs;
		DirectoryEntryAccess *dirEntry;
		static Mtfs *instance;
		static std::thread *thr;


		Mtfs();

		static void loop();


	public:
		static Mtfs *getInstance();

		static void start();

		static void join();

		static bool migrationIsValid(const int migration);

		static Rule *buildRule(int migration, rapidjson::Value &value);

	};

}  // namespace mtfs
#endif
