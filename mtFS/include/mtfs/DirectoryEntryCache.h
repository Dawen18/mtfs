#ifndef FILESTORAGE_DIRECTORY_ENTRY_CACHE_H
#define FILESTORAGE_DIRECTORY_ENTRY_CACHE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>
#include <mtfs/structs.h>

namespace mtfs {
	class DirectoryEntryCache {
	public:
		virtual bool replaceDirEntry(ident_st inodeId, int blockNb, uint8_t *block)=0;

	};

}  // namespace mtfs
#endif
