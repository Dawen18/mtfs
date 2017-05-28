#ifndef FILESTORAGE_INODE_ACCES_H
#define FILESTORAGE_INODE_ACCES_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>
#include <mtfs/structs.h>
#include "structs.h"

namespace mtfs {
	class InodeAcces {
	public:
		virtual bool addInode(inode_t &inode)=0;

		virtual bool delInode(ident_t inodeId)=0;

		virtual bool getInode(ident_t inodeId, inode_t &inode)=0;

		virtual bool setInode(ident_t inodeId, inode_t &inode)=0;

	};

}  // namespace mtfs
#endif
