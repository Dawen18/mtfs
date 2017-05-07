#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "mtfs/TimeRule.h"

namespace mtfs {
	TimeRule::TimeRule(uint64_t lowerLimit, uint64_t higerLimit) : lowerLimit(lowerLimit), higerLimit(higerLimit) {}

	bool TimeRule::satisfyRules(ruleInfo_st info) {
		return false;
	}
}  // namespace mtfs
