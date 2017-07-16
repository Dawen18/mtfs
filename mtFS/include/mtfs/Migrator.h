/// \file Migrator.h
/// \brief
/// \author David Wittwer
/// \version 0.0.1
/// \date 20.06.17

#ifndef MTFS_MIGRATOR_H
#define MTFS_MIGRATOR_H

#include <mutex>
#include <condition_variable>
#include "PoolManager.h"

namespace mtfs {
	class Migrator {
	private:
		static const int MIGRATION_DELAY=10;

	public:
		struct info_st {
			std::mutex *endMutex;
			std::condition_variable condV;
			bool end;
			PoolManager *poolManager;

			info_st() : endMutex(new std::mutex), end(false) {};

			~info_st() {
				delete endMutex;
			}
		};

		static void main(info_st *infos);
	};
}


#endif //MTFS_MIGRATOR_H
