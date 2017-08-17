/**
 * \file Migrator.cpp
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
		Logger::getInstance()->log("MIGRATOR", "start inode migration", Logger::L_DEBUG);
		infos->poolManager->doMigration(blockType::INODE);
		Logger::getInstance()->log("MIGRATOR", "start dirblock migration", Logger::L_DEBUG);
		infos->poolManager->doMigration(blockType::DIR_BLOCK);
		Logger::getInstance()->log("MIGRATOR", "start datablock migration", Logger::L_DEBUG);
		infos->poolManager->doMigration(blockType::DATA_BLOCK);
		Logger::getInstance()->log("MIGRATOR", "end migration", Logger::L_DEBUG);

		lk.lock();
	}
	lk.unlock();

}

