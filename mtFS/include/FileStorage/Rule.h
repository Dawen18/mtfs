#ifndef FILESTORAGE_RULE_H
#define FILESTORAGE_RULE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>
#include <FileStorage/structs.h>

namespace FileStorage {
	class Rule {
	public:
		virtual bool satisfyRules(ruleInfo_st info)=0;

	};

}  // namespace FileStorage
#endif
