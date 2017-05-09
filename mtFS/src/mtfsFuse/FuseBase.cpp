
#include "mtfsFuse/FuseBase.h"

namespace mtfsFuse {

	FuseBase::FuseBase() {
		callbacks = FuseCallback::getInstance();
	}

	void FuseBase::run(int argc, char **argv) {
		if (!runPrepare(argc,argv))
			return;

		callbacks->setBase(this);

	}


}  // namespace mtfsFuse
