#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "FileStorage/Volume.h"

using namespace std;

namespace FileStorage {

	bool Volume::setTimeLimits(int low, int high) {
		return false;
	}

	void Volume::getBlockInfo(uint64_t blockId, blockInfo_t &info) {
	}

	void Volume::setBlockInfo(uint64_t blockId, blockInfo_t &info) {
	}

	std::vector<ident_t> Volume::getBlocksBelowLimit() {
		vector<ident_t> blocks;

		return blocks;
	}

	std::vector<ident_t> Volume::getBlocksAboveLimit() {
		vector<ident_t> blocks;

		return blocks;
	}

	std::vector<ident_t> Volume::getInodesBelowLimit() {
		vector<ident_t> blocks;

		return blocks;
	}

	std::vector<ident_t> Volume::getInodesAboveLimit() {
		vector<ident_t> blocks;

		return blocks;
	}

	bool Volume::uptadeLastAcces(uint64_t id, std::map<uint64_t, std::vector<uint64_t>> map) {
		return false;
	}
}  // namespace FileStorage
