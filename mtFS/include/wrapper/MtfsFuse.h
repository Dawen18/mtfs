/**
 * \file MtfsFuse.h
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

#ifndef MTFSFUSE_MTFS_FUSE_H
#define MTFSFUSE_MTFS_FUSE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <cassert>

#include "wrapper/FuseBase.h"

namespace wrapper {
	class MtfsFuse : public FuseBase {

	public:

		~MtfsFuse() override;

	protected:
		bool runPrepare(int argc, char **argv) override;


		void init(void *userdata, fuse_conn_info *conn) override;

		void destroy(void *userdata) override;

		void lookup(fuse_req_t req, fuse_ino_t parent, const char *name) override;

		void getAttr(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) override;

		void setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, fuse_file_info *fi) override;

		void mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev) override;

		void open(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) override;

		void release(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) override;

		void opendir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) override;

		void access(fuse_req_t req, fuse_ino_t ino, int mask) override;

		void readdirplus(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) override;

		void releasedir(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) override;

		void readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) override;

		void mkdir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode) override;

		void
		write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, fuse_file_info *fi) override;

		void write_buf(fuse_req_t req, fuse_ino_t ino, fuse_bufvec *bufv, off_t off, fuse_file_info *fi) override;

		void read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, fuse_file_info *fi) override;

		void unlink(fuse_req_t req, fuse_ino_t parent, const char *name) override;

		void rmdir(fuse_req_t req, fuse_ino_t parent, const char *name) override;
	};

}  // namespace wrapper
#endif
