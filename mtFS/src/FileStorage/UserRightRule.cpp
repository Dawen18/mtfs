#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "mtfs/UserRightRule.h"

namespace mtfs {

	void UserRightRule::addAllowUid(uid_t uid) {
		uidAllowed.push_back(uid);
	}

	void UserRightRule::addDenyUid(uid_t uid) {
		uidDenied.push_back(uid);
	}

	void UserRightRule::addAllowGid(gid_t gid) {
		gidAllowed.push_back(gid);
	}

	void UserRightRule::addDenyGid(gid_t gid) {
		gidDenied.push_back(gid);
	}

	bool UserRightRule::satisfyRules(ruleInfo_st info) {
		return false;
	}
}  // namespace mtfs
