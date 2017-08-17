/**
 * \file UserRightRule.h
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
