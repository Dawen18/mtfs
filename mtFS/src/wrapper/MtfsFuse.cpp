/**
 * \file MtfsFuse.cpp
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

#include <string>
#include <mtfs/Mtfs.h>

#include "wrapper/MtfsFuse.h"


namespace wrapper {


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

	void MtfsFuse::mkdir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode) {
		mtfs::Mtfs::getInstance()->mkdir(req, parent, name, mode);
	}

	void MtfsFuse::unlink(fuse_req_t req, fuse_ino_t parent, const char *name) {
		mtfs::Mtfs::getInstance()->unlink(req, parent, name);
	}

	void MtfsFuse::rmdir(fuse_req_t req, fuse_ino_t parent, const char *name) {
		mtfs::Mtfs::getInstance()->rmdir(req, parent, name);
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

	void MtfsFuse::write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, fuse_file_info *fi) {
		mtfs::Mtfs::getInstance()->write(req, ino, buf, size, off, fi);
	}

	void MtfsFuse::write_buf(fuse_req_t req, fuse_ino_t ino, fuse_bufvec *bufv, off_t off, fuse_file_info *fi) {
		mtfs::Mtfs::getInstance()->write_buf(req, ino, bufv, off, fi);
	}

	void MtfsFuse::read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) {
		mtfs::Mtfs::getInstance()->read(req, ino, size, off, fi);
	}

}  // namespace wrapper
