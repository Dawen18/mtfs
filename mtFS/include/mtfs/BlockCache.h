#ifndef FILESTORAGE_BLOCK_CACHE_H
#define FILESTORAGE_BLOCK_CACHE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>
#include <mtfs/structs.h>


namespace mtfs {
	class BlockCache {
	public:
		virtual bool replaceBlock(ident_t blockId, uint8_t *block)=0;

	};

}  // namespace mtfs
#endif
