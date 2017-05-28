
#include "mtfsFuse/FuseBase.h"

namespace mtfsFuse {

	FuseBase::FuseBase() {
		callbacks = FuseCallback::getInstance();
	}

	FuseBase::~FuseBase() {

	}

	int FuseBase::run(int argc, char **argv) {

		struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
		struct fuse_session *se;
		struct fuse_cmdline_opts opts;
		int ret = -1;

		if (!runPrepare(argc, argv))
			return -1;
//		std::cout << "run" << std::endl;

		callbacks->setBase(this);

		if (fuse_parse_cmdline(&args, &opts) != 0)
			return 1;
		if (opts.show_help) {
			printf("usage: %s [options] <mountpoint>\n\n", argv[0]);
			fuse_cmdline_help();
			fuse_lowlevel_help();
			ret = 0;
			goto err_out1;
		} else if (opts.show_version) {
			printf("FUSE library version %s\n", fuse_pkgversion());
			fuse_lowlevel_version();
			ret = 0;
			goto err_out1;
		}

		se = fuse_session_new(&args, &FuseCallback::ops, sizeof(FuseCallback::ops), NULL);

		if (se == NULL)
			goto err_out1;

		if (fuse_set_signal_handlers(se) != 0)
			goto err_out2;

		if (fuse_session_mount(se, opts.mountpoint) != 0)
			goto err_out3;

		fuse_daemonize(opts.foreground);

		/* Block until ctrl+c or fusermount -u */
		if (opts.singlethread)
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

		return ret ? 1 : 0;
	}

	void FuseBase::init(void *userdata, fuse_conn_info *conn) {
	}

	void FuseBase::destroy(void *userdata) {

	}

	void FuseBase::lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::forget(fuse_req_t req, fuse_ino_t ino, uint64_t nlookup) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::getAttr(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, fuse_file_info *fi) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::readlink(fuse_req_t req, fuse_ino_t ino) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::mkdir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::unlink(fuse_req_t req, fuse_ino_t parent, const char *name) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::rmdir(fuse_req_t req, fuse_ino_t parent, const char *name) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::symlink(fuse_req_t req, const char *link, fuse_ino_t parent, const char *name) {
		fuse_reply_err(req, ENOSYS);
	}

	void
	FuseBase::rename(fuse_req_t req, fuse_ino_t parent, const char *name, fuse_ino_t newparent, const char *newname,
					 unsigned int flags) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::link(fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char *newname) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::open(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, fuse_file_info *fi) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::flush(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::release(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::fsync(fuse_req_t req, fuse_ino_t ino, int datasync, fuse_file_info *fi) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::opendir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::releasedir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::fsyncdir(fuse_req_t req, fuse_ino_t ino, int datasync, fuse_file_info *fi) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::statfs(fuse_req_t req, fuse_ino_t ino) {
		fuse_reply_err(req, ENOSYS);
	}

	void
	FuseBase::setxattr(fuse_req_t req, fuse_ino_t ino, const char *name, const char *value, size_t size, int flags) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::getxattr(fuse_req_t req, fuse_ino_t ino, const char *name, size_t size) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::listxattr(fuse_req_t req, fuse_ino_t ino, size_t size) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::removexattr(fuse_req_t req, fuse_ino_t ino, const char *name) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::access(fuse_req_t req, fuse_ino_t ino, int mask) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::create(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, fuse_file_info *fi) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::getlk(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi, struct flock *lock) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::setlk(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi, struct flock *lock, int sleep) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::bmap(fuse_req_t req, fuse_ino_t ino, size_t blocksize, uint64_t idx) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::ioctl(fuse_req_t req, fuse_ino_t ino, int cmd, void *arg, fuse_file_info *fi, unsigned int flags,
						 const void *in_buf, size_t in_bufsz, size_t out_bufsz) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::poll(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi, fuse_pollhandle *ph) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::write_buf(fuse_req_t req, fuse_ino_t ino, fuse_bufvec *bufv, off_t off, fuse_file_info *fi) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::retrive_reply(fuse_req_t req, void *cookie, fuse_ino_t ino, off_t offset, fuse_bufvec *bufv) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::forget_multi(fuse_req_t req, size_t count, fuse_forget_data *forgets) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::flock(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi, int op) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::fallocate(fuse_req_t req, fuse_ino_t ino, int mode, off_t offset, off_t length, fuse_file_info *fi) {
		fuse_reply_err(req, ENOSYS);
	}

	void FuseBase::readdirplus(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) {
		fuse_reply_err(req, ENOSYS);
	}


}  // namespace mtfsFuse
