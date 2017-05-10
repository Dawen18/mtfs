#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>
#include <cstring>

#include "mtfsFuse/MtfsFuse.h"

static const std::string hello_str = "Hello wordl!\n";
static const std::string hello_name = "hello";

static int hello_stat(fuse_ino_t ino, struct stat *stbuf) {
	stbuf->st_ino = ino;
	switch (ino) {
		case 1:
			stbuf->st_mode = S_IFDIR | 0755;
			stbuf->st_nlink = 2;
			break;

		case 2:
			stbuf->st_mode = S_IFREG | 0444;
			stbuf->st_nlink = 1;
			stbuf->st_size = hello_str.length();
			break;

		default:
			return -1;
	}
	return 0;
}


namespace mtfsFuse {




	MtfsFuse::~MtfsFuse() {

	}

	bool MtfsFuse::runPrepare(int argc, char **argv) {
		return true;
	}


	void MtfsFuse::lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
		struct fuse_entry_param e;

		if (parent != 1 || strcmp(name, hello_name.c_str()) != 0)
			fuse_reply_err(req, ENOENT);
		else {
			memset(&e, 0, sizeof(e));
			e.ino = 2;
			e.attr_timeout = 1.0;
			e.entry_timeout = 1.0;
			hello_stat(e.ino, &e.attr);

			fuse_reply_entry(req, &e);
		}

		threadQueue->push("lookup");
	}

	void MtfsFuse::getAttr(fuse_req_t req, fuse_ino_t ino, fuse_file_info *fi) {
		struct stat stbuf;

		(void) fi;

		memset(&stbuf, 0, sizeof(stbuf));
		if (hello_stat(ino, &stbuf) == -1)
			fuse_reply_err(req, ENOENT);
		else
			fuse_reply_attr(req, &stbuf, 1.0);

		threadQueue->push("getAttr");
	}

	void MtfsFuse::setThreadQueue(ThreadQueue<std::string> *threadQueue) {
		MtfsFuse::threadQueue = threadQueue;
	}

}  // namespace mtfsFuse
