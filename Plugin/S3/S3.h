/// \file S3.h
/// \brief
/// \author David Wittwer
/// \version 0.0.1
/// \date 01.08.17

#ifndef MTFS_S3_H
#define MTFS_S3_H

#include <pluginSystem/Plugin.h>
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>

namespace pluginSystem {
	class S3 : public Plugin {
	public:

		static constexpr const char *NAME = "s3";

		S3();

		std::string getName() override;

		bool attach(std::map<std::string, std::string> params) override;

		bool detach() override;

		int add(uint64_t *id, const mtfs::blockType &type) override;

		int del(const uint64_t &id, const mtfs::blockType &type) override;

		int get(const uint64_t &id, void *data, const mtfs::blockType &type, bool metas) override;

		int put(const uint64_t &id, const void *data, const mtfs::blockType &type, bool metas) override;

		bool getSuperblock(mtfs::superblock_t &superblock) override;

		bool putSuperblock(const mtfs::superblock_t &superblock) override;

	private:
		static constexpr const char *INODE_PREFIX = "inodes/";
		static constexpr const char *DIRECTORY_PREFIX = "directories/";
		static constexpr const char *DATA_PREFIX = "datas/";

		Aws::S3::S3Client *s3Client;
		Aws::SDKOptions *options;
		std::string home;
		size_t blocksize;
		Aws::String bucket;

		std::mutex inodeMutex;
		std::vector<uint64_t> freeInodes;
		uint64_t nextFreeInode;

		std::mutex dirBlockMutex;
		std::vector<uint64_t> freeDirectory;
		uint64_t nextFreeDirectory;

		std::mutex blockMutex;
		std::vector<uint64_t> freeDatas;
		uint64_t nextFreeData;

		int writeTmpFile(const std::string &filename, const void *data, const mtfs::blockType &type);

		int writeInode(const std::string &filename, const mtfs::inode_t *inode);

		int writeDirBlock(const std::string &filename, const mtfs::dirBlock_t *dirBlock);

		int writeDataBlock(const std::string &filename, const uint8_t *datas);

		int writeSuperblock(const std::string &filename, const mtfs::superblock_t *superblock);

		int dlObj(const std::string &filename, const std::string &key);

		int upObj(const std::string &filename, const std::string &key);

		bool dirExists(std::string path);

		void initIds();

		void writeMetas();

		void delObj(const std::string &key);
	};
}

#endif //MTFS_S3_H
