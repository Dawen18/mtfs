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

	void MtfsFuse::getAttr(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		mtfs::Mtfs::getInstance()->getAttr(req, ino, fi);
	}

	void MtfsFuse::lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
		mtfs::Mtfs::getInstance()->lookup(req, parent, name);
	}

	void MtfsFuse::access(fuse_req_t req, fuse_ino_t ino, int mask) {
		mtfs::Mtfs::getInstance()->access(req, ino, mask);
	}

	void MtfsFuse::opendir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		mtfs::Mtfs::getInstance()->opendir(req, ino, fi);
	}

	void MtfsFuse::mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev) {
		mtfs::Mtfs::getInstance()->mknod(req, parent, name, mode, rdev);
	}
}  // namespace mtfsFuse
