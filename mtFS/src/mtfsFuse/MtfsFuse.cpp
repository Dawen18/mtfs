#include <string>
#include <mtfs/Mtfs.h>

#include "mtfsFuse/MtfsFuse.h"


namespace mtfsFuse {


	MtfsFuse::~MtfsFuse() {

	}

	bool MtfsFuse::runPrepare(int argc, char **argv) {
		(void) argc, argv;
		return true;
	}

	void MtfsFuse::init(void *userdata, fuse_conn_info *conn) {
		mtfs::Mtfs::getInstance()->init(userdata, conn);
	}

	void MtfsFuse::destroy(void *userdata) {
		mtfs::Mtfs::getInstance()->destroy(userdata);
	}

	void MtfsFuse::lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
		mtfs::Mtfs::getInstance()->lookup(req, parent, name);
	}

	void MtfsFuse::getAttr(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		mtfs::Mtfs::getInstance()->getAttr(req, ino, fi);
	}

	void MtfsFuse::setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, fuse_file_info *fi) {
		mtfs::Mtfs::getInstance()->setAttr(req, ino, attr, to_set, fi);
	}

	void MtfsFuse::mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev) {
		mtfs::Mtfs::getInstance()->mknod(req, parent, name, mode, rdev);
	}

	void MtfsFuse::open(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		mtfs::Mtfs::getInstance()->open(req, ino, fi);
	}

	void MtfsFuse::release(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		mtfs::Mtfs::getInstance()->release(req, ino, fi);
	}

	void MtfsFuse::opendir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		mtfs::Mtfs::getInstance()->opendir(req, ino, fi);
	}

	void MtfsFuse::access(fuse_req_t req, fuse_ino_t ino, int mask) {
		mtfs::Mtfs::getInstance()->access(req, ino, mask);
	}

	void MtfsFuse::readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) {
		mtfs::Mtfs::getInstance()->readdir(req, ino, size, off, fi);
	}

	void MtfsFuse::readdirplus(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) {
		mtfs::Mtfs::getInstance()->readdirplus(req, ino, size, off, fi);
	}

	void MtfsFuse::releasedir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		mtfs::Mtfs::getInstance()->releasedir(req, ino, fi);
	}

}  // namespace mtfsFuse
