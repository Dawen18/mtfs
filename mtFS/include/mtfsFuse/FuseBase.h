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

		void run(int argc, char **argv);


	protected:
		friend class FuseCallback;

		virtual bool runPrepare(int argc, char **argv)=0;

		virtual void init(void *userdata, fuse_conn_info *conn)=0;

		virtual void destroy(void *userdata)=0;

		virtual void lookup(fuse_req_t req, fuse_ino_t parent, const char *name)=0;

		virtual void forget(fuse_req_t req, fuse_ino_t ino, uint64_t nlookup)=0;

		virtual void getAttr(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi)=0;

		virtual void setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, fuse_file_info *fi)=0;

		virtual void readlink(fuse_req_t req, fuse_ino_t ino)=0;

		virtual void mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev)=0;

		virtual void mkdir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode)=0;

		virtual void unlink(fuse_req_t req, fuse_ino_t parent, const char *name)=0;

		virtual void rmdir(fuse_req_t req, fuse_ino_t parent, const char *name)=0;

		virtual void symlink(fuse_req_t req, const char *link, fuse_ino_t parent, const char *name)=0;

		virtual void
		rename(fuse_req_t req, fuse_ino_t parent, const char *name, fuse_ino_t newparent, const char *newname,
			   unsigned int flags)=0;

		virtual void link(fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char *newname)=0;

		virtual void open(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi)=0;

		virtual void read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi)=0;

		virtual void
		write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, fuse_file_info *fi)=0;

		virtual void flush(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi)=0;

		virtual void release(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi)=0;

		virtual void fsync(fuse_req_t req, fuse_ino_t ino, int datasync, fuse_file_info *fi)=0;

		virtual void opendir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi)=0;

		virtual void readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi)=0;

		virtual void releasedir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi)=0;

		virtual void fsyncdir(fuse_req_t req, fuse_ino_t ino, int datasync, fuse_file_info *fi)=0;

		virtual void statfs(fuse_req_t req, fuse_ino_t ino)=0;

		virtual void
		setxattr(fuse_req_t req, fuse_ino_t ino, const char *name, const char *value, size_t size, int flags)=0;

		virtual void getxattr(fuse_req_t req, fuse_ino_t ino, const char *name, size_t size)=0;

		virtual void listxattr(fuse_req_t req, fuse_ino_t ino, size_t size)=0;

		virtual void removexattr(fuse_req_t req, fuse_ino_t ino, const char *name)=0;

		virtual void access(fuse_req_t req, fuse_ino_t ino, int mask)=0;

		virtual void create(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, fuse_file_info *fi)=0;

		virtual void getlk(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi, struct flock *lock)=0;

		virtual void setlk(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi, struct flock *lock, int sleep)=0;

		virtual void bmap(fuse_req_t req, fuse_ino_t ino, size_t blocksize, uint64_t idx)=0;

		virtual void ioctl(fuse_req_t req, fuse_ino_t ino, int cmd, void *arg, fuse_file_info *fi, unsigned int flags,
						   const void *in_buf, size_t in_bufsz, size_t out_bufsz)=0;

		virtual void poll(fuse_req_t rer, fuse_ino_t ino, fuse_file_info *fi, fuse_pollhandle *ph)=0;

		virtual void write_buf(fuse_req_t req, fuse_ino_t ino, fuse_bufvec *bufv, off_t off, fuse_file_info *fi)=0;

		virtual void retrive_reply(fuse_req_t req, void *cookie, fuse_ino_t ino, off_t offset, fuse_bufvec *bufv)=0;

		virtual void forget_multi(fuse_req_t req, size_t count, fuse_forget_data *forgets)=0;

		virtual void flock(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi, int op)=0;

		virtual void
		fallocate(fuse_req_t req, fuse_ino_t ino, int mode, off_t offset, off_t length, fuse_file_info *fi)=0;

		virtual void readdirplus(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi)=0;

	private:
		FuseCallback *callbacks;

	};

}  // namespace mtfsFuse
#endif
