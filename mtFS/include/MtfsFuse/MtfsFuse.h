#ifndef MTFSFUSE_MTFS_FUSE_H
#define MTFSFUSE_MTFS_FUSE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "MtfsFuse/FuseBase.h"

namespace MtfsFuse
{
class MtfsFuse : public FuseBase
{
protected:
	bool runPrepare(int argc, char **argv) override;

};

}  // namespace MtfsFuse
#endif
