#ifndef MTFSFUSE_FUSE_BASE_H
#define MTFSFUSE_FUSE_BASE_H

#include <fuse3/fuse_lowlevel.h>
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "mtfsFuse/FuseCallback.h"


namespace mtfsFuse {
	class FuseCallback;

	class FuseBase {

	public:
		FuseBase();

		virtual ~FuseBase() = 0;

		int run(int argc, char **argv);


	protected:
		friend class FuseCallback;

		virtual bool runPrepare(int argc, char **argv)=0;

		virtual void init(void *userdata, fuse_conn_info *conn);

		virtual void destroy(void *userdata);

		virtual void lookup(fuse_req_t req, fuse_ino_t parent, const char *name);

		virtual void forget(fuse_req_t req, fuse_ino_t ino, uint64_t nlookup);

		virtual void getAttr(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi);

		virtual void setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, fuse_file_info *fi);

		virtual void readlink(fuse_req_t req, fuse_ino_t ino);

		virtual void mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev);

		virtual void mkdir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode);

		virtual void unlink(fuse_req_t req, fuse_ino_t parent, const char *name);

		virtual void rmdir(fuse_req_t req, fuse_ino_t parent, const char *name);

		virtual void symlink(fuse_req_t req, const char *link, fuse_ino_t parent, const char *name);

		virtual void
		rename(fuse_req_t req, fuse_ino_t parent, const char *name, fuse_ino_t newparent, const char *newname,
			   unsigned int flags);

		virtual void link(fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char *newname);

		virtual void open(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi);

		virtual void read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi);

		virtual void write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, fuse_file_info *fi);

		virtual void flush(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi);

		virtual void release(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi);

		virtual void fsync(fuse_req_t req, fuse_ino_t ino, int datasync, fuse_file_info *fi);

		virtual void opendir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi);

		virtual void readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi);

		virtual void releasedir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi);

		virtual void fsyncdir(fuse_req_t req, fuse_ino_t ino, int datasync, fuse_file_info *fi);

		virtual void statfs(fuse_req_t req, fuse_ino_t ino);

		virtual void
		setxattr(fuse_req_t req, fuse_ino_t ino, const char *name, const char *value, size_t size, int flags);

		virtual void getxattr(fuse_req_t req, fuse_ino_t ino, const char *name, size_t size);

		virtual void listxattr(fuse_req_t req, fuse_ino_t ino, size_t size);

		virtual void removexattr(fuse_req_t req, fuse_ino_t ino, const char *name);

		virtual void access(fuse_req_t req, fuse_ino_t ino, int mask);

		virtual void create(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, fuse_file_info *fi);

		virtual void getlk(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi, struct flock *lock);

		virtual void setlk(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi, struct flock *lock, int sleep);

		virtual void bmap(fuse_req_t req, fuse_ino_t ino, size_t blocksize, uint64_t idx);

		virtual void ioctl(fuse_req_t req, fuse_ino_t ino, int cmd, void *arg, fuse_file_info *fi, unsigned int flags,
						   const void *in_buf, size_t in_bufsz, size_t out_bufsz);

		virtual void poll(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi, fuse_pollhandle *ph);

		virtual void write_buf(fuse_req_t req, fuse_ino_t ino, fuse_bufvec *bufv, off_t off, fuse_file_info *fi);

		virtual void retrive_reply(fuse_req_t req, void *cookie, fuse_ino_t ino, off_t offset, fuse_bufvec *bufv);

		virtual void forget_multi(fuse_req_t req, size_t count, fuse_forget_data *forgets);

		virtual void flock(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi, int op);

		virtual void
		fallocate(fuse_req_t req, fuse_ino_t ino, int mode, off_t offset, off_t length, fuse_file_info *fi);

		virtual void readdirplus(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi);

	private:
		FuseCallback *callbacks;

	};

}  // namespace mtfsFuse
#endif
