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
	public:
		static constexpr const char *ALLOW_USER = "allowUsers";
		static constexpr const char *DENY_USER = "denyUsers";
		static constexpr const char *ALLOW_GROUP = "allowGroups";
		static constexpr const char *DENY_GROUP = "denyGroups";

	private:
		std::vector<uid_t> uidAllowed;

		std::vector<uid_t> uidDenied;

		std::vector<gid_t> gidAllowed;

		std::vector<gid_t> gidDenied;

	public:
		static int copyConfig(rapidjson::Document &source, rapidjson::Value &destination,
							  rapidjson::Document::AllocatorType &allocator);

		static int rulesAreValid(const rapidjson::Value &value);

		void addAllowUid(uid_t uid);

		void addDenyUid(uid_t uid);

		void addAllowGid(gid_t gid);

		void addDenyGid(gid_t gid);

		bool toJson(rapidjson::Value &json, rapidjson::Document::AllocatorType &allocator) override;

		bool satisfyRules(ruleInfo_st info) override;

	};

}  // namespace mtfs
#endif
