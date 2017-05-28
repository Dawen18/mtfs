#ifndef FILESTORAGE_TIME_RULE_H
#define FILESTORAGE_TIME_RULE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "mtfs/Rule.h"

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
	};

}  // namespace mtfs
#endif
