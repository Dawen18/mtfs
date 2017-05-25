#include "mtfs/UserRightRule.h"

namespace mtfs {
	int UserRightRule::copyConfig(rapidjson::Document &source, rapidjson::Value &destination,
								  rapidjson::Document::AllocatorType &allocator) {
//		TODO copy config
		if (source.HasMember(ALLOW_USER))
			destination.AddMember(rapidjson::StringRef(ALLOW_USER), source[ALLOW_USER], allocator);

		if (source.HasMember(DENY_USER))
			destination.AddMember(rapidjson::StringRef(DENY_USER), source[DENY_USER], allocator);

		if (source.HasMember(ALLOW_GROUP))
			destination.AddMember(rapidjson::StringRef(ALLOW_GROUP), source[ALLOW_GROUP], allocator);

		if (source.HasMember(DENY_GROUP))
			destination.AddMember(rapidjson::StringRef(DENY_GROUP), source[DENY_GROUP], allocator);

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
