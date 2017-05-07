#ifndef FILESTORAGE_USER_GROUP_RULE_H
#define FILESTORAGE_USER_GROUP_RULE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "mtfs/Rule.h"

namespace mtfs {
	class UserRightRule : public Rule {
	private:
		std::vector<uid_t> uidAllowed;

		std::vector<uid_t> uidDenied;

		std::vector<gid_t> gidAllowed;

		std::vector<gid_t> gidDenied;

	public:
		void addAllowUid(uid_t uid);

		void addDenyUid(uid_t uid);

		void addAllowGid(gid_t gid);

		void addDenyGid(gid_t gid);

		bool satisfyRules(ruleInfo_st info) override;


	};

}  // namespace mtfs
#endif
