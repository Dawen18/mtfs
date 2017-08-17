/**
 * \file Semaphore.cpp
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

#include "utils/Semaphore.h"

using namespace std;

Semaphore::Semaphore(int val) : v(val) {}

Semaphore::~Semaphore() {
	this->cv.notify_all();
}

void Semaphore::wait() {
	unique_lock<std::mutex> lk(this->m);
	while (this->v <= 0)
		this->cv.wait(lk);
	this->v--;
	lk.unlock();
}

void Semaphore::notify() {
	unique_lock<mutex> lk(this->m);
	this->v++;
	lk.unlock();
	this->cv.notify_one();
}
/// \file Semaphore.cpp
/// \brief
/// \author David Wittwer
/// \version 0.0.1
/// \date 09.06.17

#include "utils/Semaphore.h"
