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


		void init(void *userdata, fuse_conn_info *conn) override;

		void destroy(void *userdata) override;

		void lookup(fuse_req_t req, fuse_ino_t parent, const char *name) override;

		void getAttr(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) override;

		void setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, fuse_file_info *fi) override;

		void mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev) override;

		void open(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) override;

		void release(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) override;

		void opendir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) override;

		void access(fuse_req_t req, fuse_ino_t ino, int mask) override;

		void readdirplus(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) override;

		void releasedir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) override;

		void readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) override;

		void mkdir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode) override;

		void
		write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, fuse_file_info *fi) override;

		void write_buf(fuse_req_t req, fuse_ino_t ino, fuse_bufvec *bufv, off_t off, fuse_file_info *fi) override;

		void read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) override;

		void unlink(fuse_req_t req, fuse_ino_t parent, const char *name) override;

		void rmdir(fuse_req_t req, fuse_ino_t parent, const char *name) override;
	};

}  // namespace mtfsFuse
#endif
