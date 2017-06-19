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
	class Acces {
	public:
		enum queryType {
			INODE,
			DIR_BLOCK,
			DATA_BLOCK,
		};

		virtual ~Acces() {};

		virtual int add(const ruleInfo_t &info, std::vector<ident_t> &ids, const queryType type, const int nb = 1)=0;

		virtual int del(const ident_t &id, const queryType type)=0;

		virtual int get(const ident_t &id, void *data, const queryType type)=0;

		virtual int put(const ident_t &id, const void *data, const queryType type)=0;

	};

}  // namespace mtfs
#endif
