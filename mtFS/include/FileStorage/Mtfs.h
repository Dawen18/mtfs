#ifndef FILESTORAGE_MTFS_H
#define FILESTORAGE_MTFS_H

#include <thread>

#include "FileStorage/InodeAcces.h"
#include "FileStorage/BlockAccess.h"
#include "FileStorage/DirectoryEntryAccess.h"

namespace FileStorage {
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
	};

}  // namespace FileStorage
#endif
