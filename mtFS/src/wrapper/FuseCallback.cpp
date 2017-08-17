#include <string>
#include <vector>
#include <list>
/**
 * \file FuseCallback.cpp
 * \brief
 * \author David Wittwer
 * \version 0.0.1
 * \copyright GNU Publis License V3
 *
 * This file is part of MTFS.

    MTFS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <assert.h>

#include <wrapper/FuseCallback.h>

namespace wrapper {
	FuseCallback *FuseCallback::self = 0;
	FuseBase *FuseCallback::base = 0;

	struct fuse_lowlevel_ops FuseCallback::ops =
			{
					.init = FuseCallback::init,
					.destroy=FuseCallback::destroy,
					.lookup=FuseCallback::lookup,
					.forget=FuseCallback::forget,
					.getattr=FuseCallback::getAttr,
					.setattr=FuseCallback::setattr,
					.readlink=FuseCallback::readlink,
					.mknod=FuseCallback::mknod,
					.mkdir=FuseCallback::mkdir,
					.unlink=FuseCallback::unlink,
					.rmdir=FuseCallback::rmdir,
					.symlink=FuseCallback::symlink,
					.rename=FuseCallback::rename,
					.link=FuseCallback::link,
					.open=FuseCallback::open,
					.read=FuseCallback::read,
					.write=FuseCallback::write,
					.flush=FuseCallback::flush,
					.release=FuseCallback::release,
					.fsync=FuseCallback::fsync,
					.opendir=FuseCallback::opendir,
					.readdir=FuseCallback::readdir,
					.releasedir=FuseCallback::releasedir,
					.fsyncdir=FuseCallback::fsyncdir,
					.statfs=FuseCallback::statfs,
					.setxattr=FuseCallback::setxattr,
					.getxattr=FuseCallback::getxattr,
					.listxattr=FuseCallback::listxattr,
					.removexattr=FuseCallback::removexattr,
					.access=FuseCallback::access,
					.create=FuseCallback::create,
					.getlk=FuseCallback::getlk,
					.setlk=FuseCallback::setlk,
					.bmap=FuseCallback::bmap,
					.ioctl=FuseCallback::ioctl,
					.poll=FuseCallback::poll,
					.write_buf=FuseCallback::write_buf,
					.retrieve_reply=FuseCallback::retrive_reply,
					.forget_multi=FuseCallback::forget_multi,
					.flock=FuseCallback::flock,
					.fallocate=FuseCallback::fallocate,
					.readdirplus=FuseCallback::readdirplus,
			};

	FuseCallback::FuseCallback() {
		this->base = 0;
	}

	FuseCallback *FuseCallback::getInstance() {
		if (!self)
			self = new FuseCallback();

		return self;
	}

	void FuseCallback::setBase(FuseBase *base) {
		this->base = base;
	}

	void FuseCallback::init(void *userdata, struct fuse_conn_info *conn) {
		if (base)
			base->init(userdata, conn);
	}

	void FuseCallback::destroy(void *userdata) {
		if (base)
			base->destroy(userdata);
	}

	void FuseCallback::lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
		if (base)
			base->lookup(req, parent, name);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::forget(fuse_req_t req, fuse_ino_t ino, uint64_t nlookup) {
		if (base)
			base->forget(req, ino, nlookup);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::getAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
		if (base)
			base->getAttr(req, ino, fi);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void
	FuseCallback::setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi) {
		if (base)
			base->setattr(req, ino, attr, to_set, fi);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::readlink(fuse_req_t req, fuse_ino_t ino) {
		if (base)
			base->readlink(req, ino);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev) {
		if (base)
			base->mknod(req, parent, name, mode, rdev);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::mkdir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode) {
		if (base)
			base->mkdir(req, parent, name, mode);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::unlink(fuse_req_t req, fuse_ino_t parent, const char *name) {
		if (base)
			base->unlink(req, parent, name);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::rmdir(fuse_req_t req, fuse_ino_t parent, const char *name) {
		if (base)
			base->rmdir(req, parent, name);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::symlink(fuse_req_t req, const char *link, fuse_ino_t parent, const char *name) {
		if (base)
			base->symlink(req, link, parent, name);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void
	FuseCallback::rename(fuse_req_t req, fuse_ino_t parent, const char *name, fuse_ino_t newparent, const char *newname,
						 unsigned int flags) {
		if (base)
			base->rename(req, parent, name, newparent, newname, flags);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::link(fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char *newname) {
		if (base)
			base->link(req, ino, newparent, newname);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
		if (base)
			base->open(req, ino, fi);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi) {
		if (base)
			base->read(req, ino, size, off, fi);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off,
							 struct fuse_file_info *fi) {
		if (base)
			base->write(req, ino, buf, size, off, fi);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::flush(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
		if (base)
			base->flush(req, ino, fi);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
		if (base)
			base->release(req, ino, fi);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::fsync(fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi) {
		if (base)
			base->fsync(req, ino, datasync, fi);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
		if (base)
			base->opendir(req, ino, fi);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi) {
		if (base)
			base->readdir(req, ino, size, off, fi);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
		if (base)
			base->releasedir(req, ino, fi);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::fsyncdir(fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info *fi) {
		if (base)
			base->fsyncdir(req, ino, datasync, fi);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::statfs(fuse_req_t req, fuse_ino_t ino) {
		if (base)
			base->statfs(req, ino);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::setxattr(fuse_req_t req, fuse_ino_t ino, const char *name, const char *value, size_t size,
								int flags) {
		if (base)
			base->setxattr(req, ino, name, value, size, flags);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::getxattr(fuse_req_t req, fuse_ino_t ino, const char *name, size_t size) {
		if (base)
			base->getxattr(req, ino, name, size);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::listxattr(fuse_req_t req, fuse_ino_t ino, size_t size) {
		if (base)
			base->listxattr(req, ino, size);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::removexattr(fuse_req_t req, fuse_ino_t ino, const char *name) {
		if (base)
			base->removexattr(req, ino, name);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::access(fuse_req_t req, fuse_ino_t ino, int mask) {
		if (base)
			base->access(req, ino, mask);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void
	FuseCallback::create(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi) {
		if (base)
			base->create(req, parent, name, mode, fi);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::getlk(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, struct flock *lock) {
		if (base)
			base->getlk(req, ino, fi, lock);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::setlk(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, struct flock *lock, int sleep) {
		if (base)
			base->setlk(req, ino, fi, lock, sleep);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::bmap(fuse_req_t req, fuse_ino_t ino, size_t blocksize, uint64_t idx) {
		if (base)
			base->bmap(req, ino, blocksize, idx);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::ioctl(fuse_req_t req, fuse_ino_t ino, int cmd, void *arg, struct fuse_file_info *fi,
							 unsigned int flags,
							 const void *in_buf, size_t in_bufsz, size_t out_bufsz) {
		if (base)
			base->ioctl(req, ino, cmd, arg, fi, flags, in_buf, in_bufsz, out_bufsz);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::poll(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, struct fuse_pollhandle *ph) {
		if (base)
			base->poll(req, ino, fi, ph);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::write_buf(fuse_req_t req, fuse_ino_t ino, struct fuse_bufvec *bufv, off_t off,
								 struct fuse_file_info *fi) {
		if (base)
			base->write_buf(req, ino, bufv, off, fi);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void
	FuseCallback::retrive_reply(fuse_req_t req, void *cookie, fuse_ino_t ino, off_t offset, struct fuse_bufvec *bufv) {
		if (base)
			base->retrive_reply(req, cookie, ino, offset, bufv);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::forget_multi(fuse_req_t req, size_t count, struct fuse_forget_data *forgets) {
		if (base)
			base->forget_multi(req, count, forgets);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void FuseCallback::flock(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi, int op) {
		if (base)
			base->flock(req, ino, fi, op);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void
	FuseCallback::fallocate(fuse_req_t req, fuse_ino_t ino, int mode, off_t offset, off_t length,
							struct fuse_file_info *fi) {
		if (base)
			base->fallocate(req, ino, mode, offset, length, fi);
		else
			fuse_reply_err(req, ENOSYS);
	}

	void
	FuseCallback::readdirplus(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi) {
		if (base)
			base->readdirplus(req, ino, size, off, fi);
		else
			fuse_reply_err(req, ENOSYS);
	}


}  // namespace wrapper
