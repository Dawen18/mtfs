#include <FileStorage/Cache.h>
#include <FileStorage/PoolManager.h>

#include "FileStorage/Mtfs.h"

namespace FileStorage {
	using namespace std;
	Mtfs *Mtfs::instance = 0;
	thread *Mtfs::thr = 0;

	Mtfs::Mtfs() {
//		PoolManager *poolManager = new PoolManager();
//		inodes = poolManager;
//		dirEntry = poolManager;
//		blocs = poolManager;
	}

	void Mtfs::loop() {

		Mtfs *mtfs = getInstance();
		int loc = 0;
		std::cout << "thread\n";
		for (int i = 0; i < 10; ++i) {
			std::this_thread::sleep_for(std::chrono::milliseconds(i));
			loc++;
			std::cout << loc << "\n";
		}

	}

	Mtfs *Mtfs::getInstance() {
		if (!instance)
			instance = new Mtfs();

		return instance;
	}

	void Mtfs::start() {
		thr = new std::thread(&loop);
	}

	void Mtfs::join() {
		thr->join();
	}

	bool Mtfs::migrationIsValid(const int migration) {
		return (migration == TIME_MIGRATION || migration == RIGHT_MIGRATION);
	}


}  // namespace FileStorage
