#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include <MtfsFuse/FuseCallback.h>
#include <MtfsFuse/FuseBase.h>

namespace MtfsFuse {
	FuseCallback *FuseCallback::self = 0;
	FuseBase *FuseCallback::base = 0;

	struct fuse_lowlevel_ops FuseCallback::ops;// =
//			{
//					init: FuseCallback::init,
//					destroy: FuseCallback::destroy,
//					lookup: FuseCallback::lookup,
//					forget: FuseCallback::forget,
//					getattr: FuseCallback::getAttr,
//					setattr: FuseCallback::setattr,
//					readlink: FuseCallback::readlink,
//					mknod: FuseCallback::mknod,
//					mkdir: FuseCallback::mkdir,
//					unlink: FuseCallback::unlink,
//					rmdir: FuseCallback::rmdir,
//					symlink: FuseCallback::symlink,
//					rename: FuseCallback::rename,
//					link: FuseCallback::link,
//					open: FuseCallback::open,
//					read: FuseCallback::read,
//					write: FuseCallback::write,
//					flush: FuseCallback::flush,
//					release: FuseCallback::release,
//					fsync: FuseCallback::fsync,
//					opendir: FuseCallback::opendir,
//					readdir: FuseCallback::readdir,
//					releasedir: FuseCallback::releasedir,
//					fsyncdir: FuseCallback::fsyncdir,
//					statfs: FuseCallback::statfs,
//					getxattr: FuseCallback::getxattr,
//					setxattr: FuseCallback::setxattr,
//					listxattr: FuseCallback::listxattr,
//					removexattr: FuseCallback::removexattr,
//					access: FuseCallback::access,
//					create: FuseCallback::create,
//					getlk: FuseCallback::getlk,
//					setlk: FuseCallback::setlk,
//					bmap: FuseCallback::bmap,
//					ioctl: FuseCallback::ioctl,
//					poll: FuseCallback::poll,
//					write_buf: FuseCallback::write_buf,
//					retrieve_reply: FuseCallback::retrive_reply,
//					forget_multi: FuseCallback::forget_multi,
//					flock: FuseCallback::flock,
//					fallocate: FuseCallback::fallocate,
//					readdirplus: FuseCallback::readdirplus,
//			};

	FuseCallback::FuseCallback() {
		ops.init = FuseCallback::init;
		ops.destroy = FuseCallback::destroy;
		ops.lookup = FuseCallback::lookup;
		ops.forget = FuseCallback::forget;
		ops.getattr = FuseCallback::getAttr;
		ops.setattr = FuseCallback::setattr;
		ops.readlink = FuseCallback::readlink;
		ops.mknod = FuseCallback::mknod;
		ops.mkdir = FuseCallback::mkdir;
		ops.unlink = FuseCallback::unlink;
		ops.rmdir = FuseCallback::rmdir;
		ops.symlink = FuseCallback::symlink;
		ops.rename = FuseCallback::rename;
		ops.link = FuseCallback::link;
		ops.open = FuseCallback::open;
		ops.read = FuseCallback::read;
		ops.write = FuseCallback::write;
		ops.flush = FuseCallback::flush;
		ops.release = FuseCallback::release;
		ops.fsync = FuseCallback::fsync;
		ops.opendir = FuseCallback::opendir;
		ops.readdir = FuseCallback::readdir;
		ops.releasedir = FuseCallback::releasedir;
		ops.fsyncdir = FuseCallback::fsyncdir;
		ops.statfs = FuseCallback::statfs;
		ops.getxattr = FuseCallback::getxattr;
		ops.setxattr = FuseCallback::setxattr;
		ops.listxattr = FuseCallback::listxattr;
		ops.removexattr = FuseCallback::removexattr;
		ops.access = FuseCallback::access;
		ops.create = FuseCallback::create;
		ops.getlk = FuseCallback::getlk;
		ops.setlk = FuseCallback::setlk;
		ops.bmap = FuseCallback::bmap;
		ops.ioctl = FuseCallback::ioctl;
		ops.poll = FuseCallback::poll;
		ops.write_buf = FuseCallback::write_buf;
		ops.retrieve_reply = FuseCallback::retrive_reply;
		ops.forget_multi = FuseCallback::forget_multi;
		ops.flock = FuseCallback::flock;
		ops.fallocate = FuseCallback::fallocate;
		ops.readdirplus = FuseCallback::readdirplus;

		this->base = 0;
		return;
	}

	FuseCallback *FuseCallback::getInstance() {
		if (!self)
			self = new FuseCallback();

		return self;
	}

	void FuseCallback::setBase(FuseBase *base) {
		this->base = base;

		return;
	}

	void FuseCallback::init(void *userdata, struct fuse_conn_info *conn) {
		if (base)
			base->init(userdata, conn);

		return;
	}

	void FuseCallback::destroy(void *userdata) {
		if (base)
			base->destroy(userdata);

		return;
	}

	void FuseCallback::lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
		if (base)
			base->lookup(req,parent,name);
		else
			fuse_reply_err(req, ENOSYS);
		return;
	}

	void FuseCallback::forget(fuse_req_t req, fuse_ino_t ino, std::uint64_t nlookup) {
		return;
	}

	void FuseCallback::getAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
		return;
	}

	void
	FuseCallback::setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi) {
		return;
	}

	void FuseCallback::readlink(fuse_req_t req, fuse_ino_t ino) {
		return;
	}

	void FuseCallback::mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev) {
		return;
	}

	void FuseCallback::mkdir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode) {
		return;
	}

	void FuseCallback::unlink(fuse_req_t req, fuse_ino_t parent, const char *name) {
		return;
	}

	void FuseCallback::rmdir(fuse_req_t req, fuse_ino_t parent, const char *name) {
		return;
	}

	void FuseCallback::symlink(fuse_req_t req, const char *link, fuse_ino_t parent, const char *name) {
		return;
	}

	void FuseCallback::rename(fuse_req_t req, fuse_ino_t parent, const char *name,
							  fuse_ino_t newparent, const char *newname, unsigned int flags) {
		return;
	}

	void FuseCallback::link(fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char *newname) {
		return;
	}

	void FuseCallback::open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
		return;
	}

	void FuseCallback::read(fuse_req_t req, fuse_ino_t ino, std::size_t size, off_t off, struct fuse_file_info *fi) {
		return;
	}

	void FuseCallback::write(fuse_req_t req, fuse_ino_t ino, const char *buf, std::size_t size, off_t off,
							 struct fuse_file_info *fi) {
		return;
	}

	void FuseCallback::flush(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
		return;
	}

	void FuseCallback::release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
		return;
	}

	void FuseCallback::fsync(fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi) {
		return;
	}

	void FuseCallback::opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
		return;
	}

	void FuseCallback::readdir(fuse_req_t req, fuse_ino_t ino, std::size_t size, off_t off, struct fuse_file_info *fi) {
		return;
	}

	void FuseCallback::releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
		return;
	}

	void FuseCallback::fsyncdir(fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi) {
		return;
	}

	void FuseCallback::statfs(fuse_req_t req, fuse_ino_t ino) {
		return;
	}

	void FuseCallback::setxattr(fuse_req_t req, fuse_ino_t ino, const char *name, const char *value, std::size_t size,
								int flags) {
		return;
	}

	void FuseCallback::getxattr(fuse_req_t req, fuse_ino_t ino, const char *name, std::size_t size) {
		return;
	}

	void FuseCallback::listxattr(fuse_req_t req, fuse_ino_t ino, std::size_t size) {
		return;
	}

	void FuseCallback::removexattr(fuse_req_t req, fuse_ino_t ino, const char *name) {
		return;
	}

	void FuseCallback::access(fuse_req_t req, fuse_ino_t ino, int mask) {
		return;
	}

	void
	FuseCallback::create(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi) {
		return;
	}

	void FuseCallback::getlk(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, struct flock *lock) {
		return;
	}

	void FuseCallback::setlk(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, struct flock *lock, int sleep) {
		return;
	}

	void FuseCallback::bmap(fuse_req_t req, fuse_ino_t ino, std::size_t blocksize, std::uint64_t idx) {
		return;
	}

	void FuseCallback::ioctl(fuse_req_t req, fuse_ino_t ino, int cmd, void *arg, struct fuse_file_info *fi,
							 unsigned int flags,
							 const void *in_buf, std::size_t in_bufsz, std::size_t out_bufsz) {
		return;
	}

	void FuseCallback::poll(fuse_req_t rer, fuse_ino_t ino, struct fuse_file_info *fi, struct fuse_pollhandle *ph) {
		return;
	}

	void FuseCallback::write_buf(fuse_req_t req, fuse_ino_t ino, struct fuse_bufvec *bufv, off_t off,
								 struct fuse_file_info *fi) {
		return;
	}

	void
	FuseCallback::retrive_reply(fuse_req_t req, void *cookie, fuse_ino_t ino, off_t offset, struct fuse_bufvec *bufv) {
		return;
	}

	void FuseCallback::forget_multi(fuse_req_t req, std::size_t count, struct fuse_forget_data *forgets) {
		return;
	}

	void FuseCallback::flock(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, int op) {
		return;
	}

	void
	FuseCallback::fallocate(fuse_req_t req, fuse_ino_t ino, int mode, off_t offset, off_t length,
							struct fuse_file_info *fi) {
		return;
	}

	void
	FuseCallback::readdirplus(fuse_req_t req, fuse_ino_t ino, std::size_t size, off_t off, struct fuse_file_info *fi) {
		return;
	}


}  // namespace MtfsFuse
