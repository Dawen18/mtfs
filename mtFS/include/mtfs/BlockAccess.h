#ifndef FILESTORAGE_BLOCK_ACCESS_H
#define FILESTORAGE_BLOCK_ACCESS_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include <mtfs/structs.h>

namespace mtfs {
	class BlockAccess {
	public:
		virtual bool addBlock(inode_st &inode)=0;

		virtual bool delBlock(inode_st &inode)=0;

		virtual bool getBlock(inode_st &inode, int blockNumber, uint8_t *buffer)=0;

		virtual bool setBlock(inode_st &inode, int blockNb, uint8_t *buffer)=0;

	};

}  // namespace mtfs
#endif
