/// \file Migrator.cpp
/// \brief
/// \author David Wittwer
/// \version 0.0.1
/// \date 20.06.17

#include <zconf.h>
#include <iostream>
#include <utils/Logger.h>
#include "mtfs/Migrator.h"

#define MIGRATION_DELAY chrono::seconds(30)

using namespace std;

void mtfs::Migrator::main(info_st *infos) {

	unique_lock<mutex> lk(*infos->endMutex);
	while (!infos->end) {
		lk.unlock();

		lk.lock();
		infos->condV.wait_for(lk, MIGRATION_DELAY);
		if (infos->end)
			break;
		lk.unlock();

//		Do migration;
		Logger::getInstance()->log("MIGRATOR", "start migration", Logger::L_DEBUG);
		infos->poolManager->doMigration();
		Logger::getInstance()->log("MIGRATOR", "end migration", Logger::L_DEBUG);

		lk.lock();
	}
	lk.unlock();

}

