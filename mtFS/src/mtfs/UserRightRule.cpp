#include "mtfs/UserRightRule.h"

namespace mtfs {
	int UserRightRule::copyConfig(rapidjson::Document &source, rapidjson::Value &destination,
								  rapidjson::Document::AllocatorType &allocator) {
//		TODO copy config
		return SUCCESS;
	}

	int UserRightRule::rulesAreValid(const rapidjson::Value &value) {
		if (value.HasMember(ALLOW_USER) || value.HasMember(ALLOW_GROUP) ||
			value.HasMember(DENY_USER) || value.HasMember(DENY_GROUP)) {
			return VALID_RULES;
		} else {
			return INVALID_RULES;
		}
	}

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
