
#include "mtfs/TimeRule.h"

namespace mtfs {
	TimeRule::TimeRule(uint64_t lowerLimit, uint64_t higerLimit) : lowerLimit(lowerLimit), higerLimit(higerLimit) {}

	bool TimeRule::satisfyRules(ruleInfo_st info) {
		return false;
	}

	bool TimeRule::rulesAreValid(const rapidjson::Value &value) {
		return (value.HasMember(TIME_LOW_LIMIT) || value.HasMember(TIME_HIGH_LIMIT));
	}
}  // namespace mtfs
