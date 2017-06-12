#ifndef FILESTORAGE_DIRECTORY_ENTRY_ACCESS_H
#define FILESTORAGE_DIRECTORY_ENTRY_ACCESS_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include <mtfs/structs.h>

namespace mtfs {
	class DirectoryBlockAccess {
	public:
		virtual ~DirectoryBlockAccess(){};

		virtual int addDirBlock(const ruleInfo_t &infos, std::vector<ident_t> &blockId, const int nb = 1)=0;

		virtual int delDirBlock(const ident_t &blockId)=0;

		virtual int getDirBlock(const ident_t &blockId, dirBlock_t &block)=0;

		virtual int putDirBlock(const ident_t &blockId, const dirBlock_t &block)=0;
	};

}  // namespace mtfs
#endif
