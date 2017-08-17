/**
 * \file TimeRule.h
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

#ifndef FILESTORAGE_TIME_RULE_H
#define FILESTORAGE_TIME_RULE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include <mtfs/Rule.h>

namespace mtfs {
	class TimeRule : public Rule {
	public:
		static constexpr const char *TIME_LOW_LIMIT = "lowLimit";
		static constexpr const char *TIME_HIGH_LIMIT = "highLimit";

	private:
		uint64_t lowerLimit;
		uint64_t higerLimit;

	public:
		static int copyConfig(rapidjson::Document &source, rapidjson::Value &destination,
							  rapidjson::Document::AllocatorType &allocator);

		static int rulesAreValid(const rapidjson::Value &value);

		TimeRule(uint64_t lowerLimit, uint64_t higerLimit);

		bool satisfyRules(ruleInfo_st info) override;

		bool toJson(rapidjson::Value &json, rapidjson::Document::AllocatorType &allocator) override;

		int configureStorage(Volume *volume) override;

		int configureStorage(Pool *pool) override;

	private:
		uint64_t now();
	};

}  // namespace mtfs
#endif
