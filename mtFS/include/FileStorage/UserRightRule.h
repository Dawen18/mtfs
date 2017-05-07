#ifndef FILESTORAGE_USER_GROUP_RULE_H
#define FILESTORAGE_USER_GROUP_RULE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "FileStorage/Rule.h"

namespace FileStorage {
	class UserRightRule : public Rule {
	private:
		std::vector<uint16_t> uidAllowed;

		std::vector<uint16_t> uidDenied;

		std::vector<uint16_t> gidAllowed;

		std::vector<uint16_t> gidDenied;


	};

}  // namespace FileStorage
#endif
