#ifndef MTFSFUSE_FUSE_CALLBACK_H
#define MTFSFUSE_FUSE_CALLBACK_H

#include <fuse3/fuse_lowlevel.h>
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include "MtfsFuse/FuseBase.h"


namespace MtfsFuse {
	class FuseBase;

	class FuseCallback {
	public:
		FuseCallback *getInstance();

		void setBase(FuseBase *base);

		static struct fuse_lowlevel_ops ops;


	private:
		FuseCallback();

		static void init(void *userdata, struct fuse_conn_info *conn);

		static void destroy(void *userdata);

		static void lookup(fuse_req_t req, fuse_ino_t parent, const char *name);

		static void forget(fuse_req_t req, fuse_ino_t ino, std::uint64_t nlookup);

		static void getAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

		static void setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi);

		static void readlink(fuse_req_t req, fuse_ino_t ino);

		static void mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev);

		static void mkdir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode);

		static void unlink(fuse_req_t req, fuse_ino_t parent, const char *name);

		static void rmdir(fuse_req_t req, fuse_ino_t parent, const char *name);

		static void symlink(fuse_req_t req, const char *link, fuse_ino_t parent, const char *name);

		static void
		rename(fuse_req_t req, fuse_ino_t parent, const char *name, fuse_ino_t newparent, const char *newname,
			   unsigned int flags);

		static void link(fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char *newname);

		static void open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

		static void read(fuse_req_t req, fuse_ino_t ino, std::size_t size, off_t off, struct fuse_file_info *fi);

		static void
		write(fuse_req_t req, fuse_ino_t ino, const char *buf, std::size_t size, off_t off, struct fuse_file_info *fi);

		static void flush(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

		static void release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

		static void fsync(fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi);

		static void opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

		static void readdir(fuse_req_t req, fuse_ino_t ino, std::size_t size, off_t off, struct fuse_file_info *fi);

		static void releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

		static void fsyncdir(fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi);

		static void statfs(fuse_req_t req, fuse_ino_t ino);

		static void
		setxattr(fuse_req_t req, fuse_ino_t ino, const char *name, const char *value, std::size_t size, int flags);

		static void getxattr(fuse_req_t req, fuse_ino_t ino, const char *name, std::size_t size);

		static void listxattr(fuse_req_t req, fuse_ino_t ino, std::size_t size);

		static void removexattr(fuse_req_t req, fuse_ino_t ino, const char *name);

		static void access(fuse_req_t req, fuse_ino_t ino, int mask);

		static void create(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi);

		static void getlk(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, struct flock *lock);

		static void setlk(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, struct flock *lock, int sleep);

		static void bmap(fuse_req_t req, fuse_ino_t ino, std::size_t blocksize, std::uint64_t idx);

		static void
		ioctl(fuse_req_t req, fuse_ino_t ino, int cmd, void *arg, struct fuse_file_info *fi, unsigned int flags,
			  const void *in_buf, std::size_t in_bufsz, std::size_t out_bufsz);

		static void poll(fuse_req_t rer, fuse_ino_t ino, struct fuse_file_info *fi, struct fuse_pollhandle *ph);

		static void
		write_buf(fuse_req_t req, fuse_ino_t ino, struct fuse_bufvec *bufv, off_t off, struct fuse_file_info *fi);

		static void retrive_reply(fuse_req_t req, void *cookie, fuse_ino_t ino, off_t offset, struct fuse_bufvec *bufv);

		static void forget_multi(fuse_req_t req, std::size_t count, struct fuse_forget_data *forgets);

		static void flock(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, int op);

		static void
		fallocate(fuse_req_t req, fuse_ino_t ino, int mode, off_t offset, off_t length, struct fuse_file_info *fi);

		static void readdirplus(fuse_req_t req, fuse_ino_t ino, std::size_t size, off_t off, struct fuse_file_info *fi);


		static FuseCallback *self;

		static FuseBase *base;

	};

}  // namespace MtfsFuse
#endif
