
#include "mtfs/TimeRule.h"

namespace mtfs {
	TimeRule::TimeRule(uint64_t lowerLimit, uint64_t higerLimit) : lowerLimit(lowerLimit),
																   higerLimit(higerLimit) {}

	bool TimeRule::satisfyRules(ruleInfo_st info) {
		uint64_t now = this->now();
		uint64_t delay = now - info.lastAccess;

		if (0 == this->lowerLimit && 0 == this->higerLimit)
			return true;
		else if (0 == this->lowerLimit)
			return delay <= this->higerLimit;
		else if (0 == this->higerLimit)
			return delay >= this->lowerLimit;
		else
			return this->lowerLimit <= delay <= this->higerLimit;
	}

	int TimeRule::copyConfig(rapidjson::Document &source, rapidjson::Value &destination,
							 rapidjson::Document::AllocatorType &allocator) {

		if (source.HasMember(TIME_LOW_LIMIT))
			destination.AddMember(rapidjson::StringRef(TIME_LOW_LIMIT), source[TIME_LOW_LIMIT], allocator);

		if (source.HasMember(TIME_HIGH_LIMIT))
			destination.AddMember(rapidjson::StringRef(TIME_HIGH_LIMIT), source[TIME_HIGH_LIMIT], allocator);

		return SUCCESS;
	}

	int TimeRule::rulesAreValid(const rapidjson::Value &value) {
		if (value.HasMember(TIME_LOW_LIMIT) || value.HasMember(TIME_HIGH_LIMIT)) {
			return VALID_RULES;
		} else {
			return INVALID_RULES;
		}
	}

	bool TimeRule::toJson(rapidjson::Value &json, rapidjson::Document::AllocatorType &allocator) {

		rapidjson::Value v;

		if (this->lowerLimit != 0) {
			v.SetUint64(this->lowerLimit);
			json.AddMember(rapidjson::StringRef(TIME_LOW_LIMIT), v, allocator);
		}

		if (this->higerLimit != 0) {
			v.SetUint64(this->higerLimit);
			json.AddMember(rapidjson::StringRef(TIME_HIGH_LIMIT), v, allocator);
		}

		return true;
	}

	uint64_t TimeRule::now() {
		return (uint64_t) time(NULL);
	}

	int TimeRule::configureStorage(Volume *volume) {
		volume->setMinDelay(this->lowerLimit);
		volume->setMaxDelay(this->higerLimit);
		volume->setIsTimeVolume(true);

		return 0;
	}

	int TimeRule::configureStorage(Pool *pool) {
		return 0;
	}

}  // namespace mtfs
