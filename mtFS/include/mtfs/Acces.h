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

		virtual int add(const ruleInfo_t &info, std::vector<ident_t> &ids, const blockType type, const size_t nb = 1)=0;

		virtual int del(const ident_t &id, const blockType type)=0;

		virtual int get(const ident_t &id, void *data, const blockType type)=0;

		virtual int put(const ident_t &id, const void *data, const blockType type)=0;

		virtual int getMetas(const ident_t &id, blockInfo_t &metas, const blockType type)=0;

		virtual int putMetas(const ident_t &id, const blockInfo_t &metas, const blockType type)=0;
	};

}  // namespace mtfs
#endif
