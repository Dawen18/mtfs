/**
 * \file Access.h
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

#ifndef FILESTORAGE_INODE_ACCES_H
#define FILESTORAGE_INODE_ACCES_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>
#include <mtfs/structs.h>

namespace mtfs {
	typedef struct ruleInfo_st ruleInfo_t;

	class Acces {
	public:

		virtual ~Acces() {};

		/**
		 * Add a block in a volume
		 *
		 * @param [in] info ruleInfo_t To choose a pool or volume in which add the block
		 * @param [out] ids Id for new blocks
		 * @param [in] type Type of block (see enum blockType)
		 * @param [in] nb Numbre of block to add.
		 *
		 * @return 0 if success else std linus error code.
		 */
		virtual int add(const ruleInfo_t &info, std::vector<ident_t> &ids, blockType type, size_t nb)=0;

		/**
		 * Dele a block in a volume
		 *
		 * @param [in] id Id of block to delete
		 * @param [in] type Type of block to delete
		 *
		 * @return 0 if success else std linus error code.
		 */
		virtual int del(const ident_t &id, blockType type)=0;

		/**
		 * Get a block in a volume
		 *
		 * @param [in] id Id of block
		 * @param [out] data Pointer on memory already allocate
		 * @param [in] type Type of block
		 *
		 * @return 0 if success else std linus error code.
		 */
		virtual int get(const ident_t &id, void *data, blockType type)=0;

		/**
		 * Put a block in a volume
		 *
		 * @param [in] id Id of block
		 * @param [in] data Pointer on memory who contains the data.
		 * @param [in] type Type of block
		 *
		 * @return 0 if success else std linus error code.
		 */
		virtual int put(const ident_t &id, const void *data, blockType type)=0;

		/**
		 * Get metas block info
		 *
		 * @param [in] id Id of block
		 * @param [out] metas Datas
		 * @param [in] type Type of block
		 *
		 * @return 0 if success else std linus error code.
		 */
		virtual int getMetas(const ident_t &id, blockInfo_t &metas, blockType type)=0;

		/**
		 * Put metas block info
		 *
		 * @param id Id of block
		 * @param metas Datas
		 * @param type Type of block
		 *
		 * @return 0 if success else std linus error code.
		 */
		virtual int putMetas(const ident_t &id, const blockInfo_t &metas, blockType type)=0;
	};

}  // namespace mtfs
#endif
