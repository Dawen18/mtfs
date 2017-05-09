#ifndef FILESTORAGE_RULE_H
#define FILESTORAGE_RULE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>
#include <mtfs/structs.h>
#include <rapidjson/schema.h>

namespace mtfs {
	class Rule {
	public:
		static constexpr const char *MIGRATION = "migration";
		static const int NO_MIGRATION = -1;
		static const int TIME_MIGRATION = 0;
		static const int RIGHT_MIGRATION = 1;

	public:
		static bool rulesAreValid(int migration, const rapidjson::Value &value);
		static Rule *buildRule(int migration, const rapidjson::Value &value);
		virtual bool satisfyRules(ruleInfo_st info)=0;

	};

}  // namespace mtfs
#endif
