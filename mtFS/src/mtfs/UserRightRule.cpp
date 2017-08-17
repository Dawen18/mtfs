/**
 * \file UserRightRule.cpp
 * \brief
 * \author David Wittwer
 * \version 0.0.1
 * \copyright GNU Publis License V3
 *
 * This file is part of MTFS.

    MTFS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pwd.h>
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
		return true;
	}

	bool UserRightRule::toJson(rapidjson::Value &json, rapidjson::Document::AllocatorType &allocator) {

		rapidjson::Value v;
		rapidjson::Value a(rapidjson::kArrayType);

		struct passwd *pw;
		char buffer[50];
		memset(buffer, 0, 50 * sizeof(char));

		if (this->uidAllowed.size() != 0) {
			for (auto &&allowed : this->uidAllowed) {
				pw = getpwuid(allowed);
				int len = sprintf(buffer, "%s", pw->pw_name);
				v.SetString(buffer, (rapidjson::SizeType) len, allocator);
				a.PushBack(v, allocator);
			}
			json.AddMember(rapidjson::StringRef(ALLOW_USER), a, allocator);
		}

		if (this->uidDenied.size() != 0) {
			for (auto &&denied : this->uidDenied) {
				pw = getpwuid(denied);
				int len = sprintf(buffer, "%s", pw->pw_name);
				v.SetString(buffer, (rapidjson::SizeType) len, allocator);
				a.PushBack(v, allocator);
			}
			json.AddMember(rapidjson::StringRef(DENY_USER), a, allocator);
		}

		if (this->gidAllowed.size() != 0) {
			for (auto &&allowed : this->gidAllowed) {
				pw = getpwuid(allowed);
				int len = sprintf(buffer, "%s", pw->pw_name);
				v.SetString(buffer, (rapidjson::SizeType) len, allocator);
				a.PushBack(v, allocator);
			}
			json.AddMember(rapidjson::StringRef(ALLOW_GROUP), a, allocator);
		}

		if (this->gidDenied.size() != 0) {
			for (auto &&denied : this->gidDenied) {
				pw = getpwuid(denied);
				int len = sprintf(buffer, "%s", pw->pw_name);
				v.SetString(buffer, (rapidjson::SizeType) len, allocator);
				a.PushBack(v, allocator);
			}
			json.AddMember(rapidjson::StringRef(DENY_GROUP), a, allocator);
		}

		return true;
	}

}  // namespace mtfs
