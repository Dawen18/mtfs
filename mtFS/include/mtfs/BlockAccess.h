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

		virtual ~BlockAccess(){};

		virtual int addBlock(const ruleInfo_t &infos, std::vector<ident_t> &ident, const int nb = 1)=0;

		virtual bool delBlock(inode_st &inode)=0;

		virtual int getBlock(const ident_t &blockId, uint8_t *buffer)=0;

		virtual int putBlock(const ident_t &blockId, uint8_t *buffer)=0;

	};

}  // namespace mtfs
#endif
