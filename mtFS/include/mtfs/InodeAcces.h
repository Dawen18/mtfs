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
		virtual ~InodeAcces(){};

		virtual int addInode(const ruleInfo_t &info, std::vector<ident_t> &idents, const int nb = 1)=0;

		virtual int delInode(const ident_t &inodeId)=0;

		virtual int getInode(const ident_t &inodeId, inode_t &inode)=0;

		virtual int putInode(const ident_t &inodeId, const inode_t &inode)=0;

	};

}  // namespace mtfs
#endif
