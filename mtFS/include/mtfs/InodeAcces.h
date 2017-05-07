#ifndef FILESTORAGE_INODE_ACCES_H
#define FILESTORAGE_INODE_ACCES_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>
#include <mtfs/structs.h>

namespace mtfs {
	class InodeAcces {
	public:
		virtual bool addInode(inode_st &inode)=0;

		virtual bool delInode(ident_st inodeId)=0;

		virtual bool getInode(ident_st inodeId, inode_st &inode)=0;

		virtual bool setInode(ident_st inodeId, inode_st &inode)=0;

	};

}  // namespace mtfs
#endif
