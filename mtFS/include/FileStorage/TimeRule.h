#ifndef FILESTORAGE_TIME_RULE_H
#define FILESTORAGE_TIME_RULE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "FileStorage/Rule.h"

namespace FileStorage {
	class TimeRule : public Rule {
	private:
		uint64_t lowerLimit;

		uint64_t higerLimit;

	};

}  // namespace FileStorage
#endif
