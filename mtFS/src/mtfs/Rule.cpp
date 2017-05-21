/// \file Rule.cpp
/// \brief Interface for Rule objects
/// \author David Wittwer
/// \version 0.0.1
/// \date 01.05.17

#include <mtfs/TimeRule.h>
#include <mtfs/UserRightRule.h>
#include <pwd.h>

namespace mtfs {

	int Rule::copyConfig(int migration, rapidjson::Document &source, rapidjson::Value &destination,
						 rapidjson::Document::AllocatorType &allocator) {
		switch (migration) {
			case TIME_MIGRATION:
				return TimeRule::copyConfig(source, destination, allocator);
			case RIGHT_MIGRATION:
				return UserRightRule::copyConfig(source, destination, allocator);
			default:
				return UNKNOW_MIGRATION;
		}
	}

	int Rule::rulesAreValid(int migration, const rapidjson::Value &value) {
		switch (migration) {
			case NO_MIGRATION:
				return SUCCESS;
			case TIME_MIGRATION:
				return TimeRule::rulesAreValid(value);
			case RIGHT_MIGRATION:
				return UserRightRule::rulesAreValid(value);
			default:
				return UNKNOW_MIGRATION;
		}
	}

	Rule *Rule::buildRule(int migration, const rapidjson::Value &value) {
		Rule *rule = nullptr;
		if (migration == NO_MIGRATION)
			rule = nullptr;
		else if (migration == TIME_MIGRATION) {
			uint64_t ll = 0, hl = 0;

			if (value.HasMember(TimeRule::TIME_LOW_LIMIT))
				ll = value[TimeRule::TIME_LOW_LIMIT].GetUint64();

			if (value.HasMember(TimeRule::TIME_HIGH_LIMIT))
				hl = value[TimeRule::TIME_HIGH_LIMIT].GetUint64();

			rule = new TimeRule(ll, hl);
		} else if (migration == RIGHT_MIGRATION) {
			UserRightRule *uRule = new UserRightRule();

			if (value.HasMember(UserRightRule::ALLOW_USER))
				for (auto &ua: value[UserRightRule::ALLOW_USER].GetArray())
					uRule->addAllowUid(getpwnam(ua.GetString())->pw_uid);

			if (value.HasMember(UserRightRule::DENY_USER))
				for (auto &ud: value[UserRightRule::DENY_USER].GetArray())
					uRule->addDenyUid(getpwnam(ud.GetString())->pw_uid);

			if (value.HasMember(UserRightRule::ALLOW_GROUP))
				for (auto &ga: value[UserRightRule::ALLOW_GROUP].GetArray())
					uRule->addAllowGid(getpwnam(ga.GetString())->pw_gid);

			if (value.HasMember(UserRightRule::DENY_GROUP))
				for (auto &ga: value[UserRightRule::DENY_GROUP].GetArray())
					uRule->addDenyGid(getpwnam(ga.GetString())->pw_gid);

			rule = uRule;
		}
		return rule;
	}
}
