/**
 * \file FuseBase.cpp
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

#include <utils/Logger.h>
#include <fstream>
#include "wrapper/FuseBase.h"

namespace wrapper {

	FuseBase::FuseBase() {
		callbacks = FuseCallback::getInstance();
	}

	FuseBase::~FuseBase() = default;

	int FuseBase::run(int argc, char **argv) {

		struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
		struct fuse_session *se;
		struct fuse_cmdline_opts opts{};
		int ret = -1;

		if (!runPrepare(argc, argv))
			return -1;
//		std::cout << "run" << std::endl;

		callbacks->setBase(this);

		if (fuse_parse_cmdline(&args, &opts) != 0)
			return 1;
		if (0 != opts.show_help) {
			std::cerr << "usage: " << argv[0] << " [options] <mountpoint> <configName>" << std::endl << std::endl;
			fuse_cmdline_help();
			fuse_lowlevel_help();
			ret = 0;
			goto err_out1;
		} else if (0 != opts.show_version) {
			std::cerr << "FUSE library version " << fuse_pkgversion() << std::endl;
			fuse_lowlevel_version();
			ret = 0;
			goto err_out1;
		}

		if (0 != opts.debug) {
			std::ofstream log("/tmp/mtfs.log");
			Logger::init(std::cerr, Logger::L_DEBUG);
		} else {
			std::ofstream log;
			log.open("/var/log/mtfs.log");
			Logger::init(log, Logger::L_ERROR);
		}

		se = fuse_session_new(&args, &FuseCallback::ops, sizeof(FuseCallback::ops), nullptr);

		if (se == nullptr)
			goto err_out1;

		if (fuse_set_signal_handlers(se) != 0)
			goto err_out2;

		if (fuse_session_mount(se, opts.mountpoint) != 0)
			goto err_out3;

		fuse_daemonize(opts.foreground);

		/* Block until ctrl+c or fusermount -u */
		if (0 != opts.singlethread)
			ret = fuse_session_loop(se);
		else
			ret = fuse_session_loop_mt(se, opts.clone_fd);

		fuse_session_unmount(se);
		err_out3:
		fuse_remove_signal_handlers(se);
		err_out2:
		fuse_session_destroy(se);
		err_out1:
		free(opts.mountpoint);
		fuse_opt_free_args(&args);

		return 0 != ret ? 1 : 0;
	}

	void FuseBase::init(void *userdata, fuse_conn_info *conn) {
		(void) userdata, conn;
	}

	void FuseBase::destroy(void *userdata) {
		(void) userdata;
	}

	void FuseBase::lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
		(void) parent, name;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::forget(fuse_req_t req, fuse_ino_t ino, uint64_t nlookup) {
		(void) ino, nlookup;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::getAttr(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		(void) ino, fi;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, fuse_file_info *fi) {
		(void) ino, attr, to_set, fi;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::readlink(fuse_req_t req, fuse_ino_t ino) {
		(void) ino;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev) {
		(void) parent, name, mode, rdev;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::mkdir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode) {
		(void) parent, name, mode;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::unlink(fuse_req_t req, fuse_ino_t parent, const char *name) {
		(void) parent, name;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::rmdir(fuse_req_t req, fuse_ino_t parent, const char *name) {
		(void) parent, name;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::symlink(fuse_req_t req, const char *link, fuse_ino_t parent, const char *name) {
		(void) link, parent, name;
		fuse_reply_err(req, ENOSYS);
	}

	void
	FuseBase::rename(fuse_req_t req, fuse_ino_t parent, const char *name, fuse_ino_t newparent, const char *newname,
					 unsigned int flags) {
		(void) parent, name, newparent, newname, flags;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::link(fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char *newname) {
		(void) ino, newparent, newname;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::open(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		(void) ino, fi;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) {
		(void) ino, size, off, fi;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, fuse_file_info *fi) {
		(void) ino, buf, size, off, fi;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::flush(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		(void) ino, fi;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::release(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		(void) ino, fi;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::fsync(fuse_req_t req, fuse_ino_t ino, int datasync, fuse_file_info *fi) {
		(void) ino, datasync, fi;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::opendir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		(void) ino, fi;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) {
		(void) ino, size, off, fi;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::releasedir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		(void) ino, fi;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::fsyncdir(fuse_req_t req, fuse_ino_t ino, int datasync, fuse_file_info *fi) {
		(void) ino, datasync, fi;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::statfs(fuse_req_t req, fuse_ino_t ino) {
		(void) ino;
		fuse_reply_err(req, ENOSYS);
	}

	void
	FuseBase::setxattr(fuse_req_t req, fuse_ino_t ino, const char *name, const char *value, size_t size, int flags) {
		(void) ino, name, value, size, flags;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::getxattr(fuse_req_t req, fuse_ino_t ino, const char *name, size_t size) {
		(void) ino, name, size;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::listxattr(fuse_req_t req, fuse_ino_t ino, size_t size) {
		(void) ino, size;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::removexattr(fuse_req_t req, fuse_ino_t ino, const char *name) {
		(void) ino, name;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::access(fuse_req_t req, fuse_ino_t ino, int mask) {
		(void) ino, mask;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::create(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, fuse_file_info *fi) {
		(void) parent, name, mode, fi;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::getlk(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi, struct flock *lock) {
		(void) ino, fi, lock;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::setlk(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi, struct flock *lock, int sleep) {
		(void) ino, fi, lock, sleep;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::bmap(fuse_req_t req, fuse_ino_t ino, size_t blocksize, uint64_t idx) {
		(void) ino, blocksize, idx;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::ioctl(fuse_req_t req, fuse_ino_t ino, int cmd, void *arg, fuse_file_info *fi, unsigned int flags,
						 const void *in_buf, size_t in_bufsz, size_t out_bufsz) {
		(void) ino, cmd, arg, fi, flags, in_buf, in_bufsz, out_bufsz;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::poll(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi, fuse_pollhandle *ph) {
		(void) ino, fi, ph;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::write_buf(fuse_req_t req, fuse_ino_t ino, fuse_bufvec *bufv, off_t off, fuse_file_info *fi) {
		(void) ino, bufv, off, fi;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::retrive_reply(fuse_req_t req, void *cookie, fuse_ino_t ino, off_t offset, fuse_bufvec *bufv) {
		(void) cookie, ino, offset, bufv;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::forget_multi(fuse_req_t req, size_t count, fuse_forget_data *forgets) {
		(void) count, forgets;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::flock(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi, int op) {
		(void) ino, fi, op;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::fallocate(fuse_req_t req, fuse_ino_t ino, int mode, off_t offset, off_t length, fuse_file_info *fi) {
		(void) ino, mode, offset, length, fi;
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::readdirplus(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) {
		(void) ino, size, off, fi;
		fuse_reply_err(req, ENOSYS);
	}


}  // namespace wrapper
