#ifndef MTFSFUSE_MTFS_FUSE_H
#define MTFSFUSE_MTFS_FUSE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>
#include <utils/ThreadQueue.h>

#include "mtfsFuse/FuseBase.h"

namespace mtfsFuse {
	class MtfsFuse : public FuseBase {

	public:

		~MtfsFuse() override;

	protected:
		bool runPrepare(int argc, char **argv) override;

		void getAttr(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) override;

		void lookup(fuse_req_t req, fuse_ino_t parent, const char *name) override;

		void init(void *userdata, fuse_conn_info *conn) override;

		void access(fuse_req_t req, fuse_ino_t ino, int mask) override;

		void opendir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) override;

		void mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev) override;

		void destroy(void *userdata) override;
	};

}  // namespace mtfsFuse
#endif
