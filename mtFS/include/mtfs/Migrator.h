/**
 * \file Migrator.h
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


#ifndef MTFS_MIGRATOR_H
#define MTFS_MIGRATOR_H

#include <mutex>
#include <condition_variable>
#include "PoolManager.h"

namespace mtfs {
	class Migrator {
	private:
		static const int MIGRATION_DELAY = 10;

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
