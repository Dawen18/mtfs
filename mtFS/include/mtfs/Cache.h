/**
 * \file Cache.h
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

#ifndef FILESTORAGE_CACHE_H
#define FILESTORAGE_CACHE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include "mtfs/Acces.h"
#include "mtfs/PoolManager.h"

namespace mtfs {
	class Cache :
			public Acces {

	public:
		virtual int store(const ident_t &id, void *data, const blockType &type)=0;

		virtual int remove(const ident_t &id, const blockType &type)=0;
	};

}  // namespace mtfs
#endif
