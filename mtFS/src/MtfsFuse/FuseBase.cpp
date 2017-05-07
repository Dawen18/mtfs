#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "MtfsFuse/FuseBase.h"

namespace MtfsFuse {

	FuseBase::FuseBase() {
		callbacks = FuseCallback::getInstance();
	}

	void FuseBase::run(int argc, char **argv) {
		if (!runPrepare(argc,argv))
			return;

		callbacks->setBase(this);

	}


}  // namespace MtfsFuse
