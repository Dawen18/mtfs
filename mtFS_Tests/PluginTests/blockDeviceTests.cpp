/**
 * @author David Wittwer
 * @date 22.04.17.
**/

#include <gtest/gtest.h>
#include <BlockDevice/BlockDevice.h>
#include <mtfs/Mtfs.h>
//#include <mtfs/structs.h>

#define BLOCK_SIZE 4096

using namespace std;
using namespace pluginSystem;

const string HOME = "/home/david/Cours/4eme/Travail_bachelor/Home/Plugins/";

TEST(BlockDevice, attachDetach) {
#ifndef DEBUG
	if (setuid(0) != 0)
		cout << "fail setuid" << endl;
#endif

	BlockDevice blockDevice;
	map<string, string> params;
	params.insert(make_pair("home", HOME));
	params.insert(make_pair("blockSize", to_string(BLOCK_SIZE)));
	params.insert(make_pair("devicePath", "/dev/sdd1"));
	params.insert(make_pair("fsType", "ext4"));
	ASSERT_TRUE(blockDevice.attach(params));
	ASSERT_TRUE(blockDevice.detach());
}

class BlockDeviceFixture : public ::testing::Test {
public:
	BlockDeviceFixture() {
		rootIdent.poolId = 0;
		rootIdent.volumeId = 0;
		rootIdent.id = 0;
	}

	virtual void SetUp() {
		map<string, string> params;
		params.insert(make_pair("home", HOME));
		params.insert(make_pair("blockSize", "4096"));
		params.insert(make_pair("devicePath", "/dev/sdd1"));
		params.insert(make_pair("fsType", "ext4"));
		blockDevice.attach(params);
	}

	virtual void TearDown() {
		blockDevice.detach();
	}

	~BlockDeviceFixture() {
	}

	BlockDevice blockDevice;
	mtfs::ident_t rootIdent;

};

TEST_F(BlockDeviceFixture, addInode) {
	uint64_t inode = 0;
	ASSERT_EQ(0, blockDevice.addInode(&inode));
	ASSERT_NE(0, inode);

	uint64_t inode2 = 0;
	ASSERT_EQ(0, blockDevice.addInode(&inode2));
	ASSERT_NE(0, inode2);
	ASSERT_GT(inode2, inode);

	uint64_t inode3 = 0;
	ASSERT_EQ(0, blockDevice.addInode(&inode3));
}

TEST_F(BlockDeviceFixture, delInode) {
	uint64_t inode;
	blockDevice.addInode(&inode);
	ASSERT_EQ(0, blockDevice.delInode(inode));

	uint64_t inode2;
	blockDevice.addInode(&inode2);
	blockDevice.addInode(&inode2);
	blockDevice.addInode(&inode2);
	blockDevice.delInode(inode);
}

TEST_F(BlockDeviceFixture, writeInode) {
	uint64_t inodeId;
	blockDevice.addInode(&inodeId);

	mtfs::ident_t oIdent;
	oIdent.id = 42;
	oIdent.volumeId = 1;
	oIdent.poolId = 1;

	mtfs::inode_t ino;
	ino.accesRight = 0666;
	ino.uid = 0;
	ino.gid = 0;
	ino.size = 1024;
	ino.linkCount = 1;
	ino.referenceId.push_back(rootIdent);
	ino.referenceId.push_back(oIdent);
	ino.atime = (unsigned long &&) time(nullptr);

	for (int i = 0; i < 4; ++i) {
		vector<mtfs::ident_t> blocks;

		for (uint j = 0; j < 3; ++j) {
			mtfs::ident_t ident = {
					.poolId = 1,
					.volumeId = j,
					.id = i * 3 + j,
			};

			blocks.push_back(ident);
		}
		ino.dataBlocks.push_back(blocks);
	}

	ASSERT_EQ(0, blockDevice.putInode(inodeId, ino));
}

TEST_F(BlockDeviceFixture, readInode) {
	mtfs::inode_t original, inode;
	memset(&original, 0, sizeof(mtfs::inode_t));
	memset(&inode, 0, sizeof(mtfs::inode_t));

	original.accesRight = 0644;
	original.uid = 1;
	original.gid = 1;
	original.size = 1024;
	original.linkCount = 1;
	original.referenceId.push_back(rootIdent);
	original.atime = (unsigned long &&) time(nullptr);

	for (int i = 0; i < 4; ++i) {
		vector<mtfs::ident_t> blocks;

		for (uint j = 0; j < 3; ++j) {
			mtfs::ident_t ident = {
					.poolId = 1,
					.volumeId = j,
					.id = i * 3 + j,
			};

			blocks.push_back(ident);
		}
		original.dataBlocks.push_back(blocks);
	}

	uint64_t inodeId;
	blockDevice.addInode(&inodeId);
	cout << "inodeId " << inodeId << "  " << endl;
	blockDevice.putInode(inodeId, original);
	ASSERT_EQ(0, blockDevice.getInode(inodeId, inode));

//	cout << original.accesRight << " " << inode.accesRight << endl;
	ASSERT_EQ(original, inode);
	original.size = 2048;
	blockDevice.putInode(inodeId, original);
	ASSERT_EQ(0, blockDevice.getInode(inodeId, inode));
}

TEST_F(BlockDeviceFixture, addBlock) {
	uint64_t block = 0;
	ASSERT_EQ(0, blockDevice.addBlock(&block));
	ASSERT_NE(0, block);

	uint64_t block2 = 0;
	ASSERT_EQ(0, blockDevice.addBlock(&block2));
	ASSERT_NE(0, block2);
	ASSERT_GT(block2, block);

	uint64_t block3 = 0;
	ASSERT_EQ(0, blockDevice.addBlock(&block3));
}

TEST_F(BlockDeviceFixture, delBlock) {
	uint64_t block;
	blockDevice.addBlock(&block);
	ASSERT_EQ(0, blockDevice.delBlock(block));

	uint64_t block2;
	blockDevice.addBlock(&block2);
	blockDevice.addBlock(&block2);
	blockDevice.addBlock(&block2);
	blockDevice.delBlock(block);
}

TEST_F(BlockDeviceFixture, writeBlock) {
	uint8_t block[BLOCK_SIZE];
	memset(block, 0, BLOCK_SIZE);
	for (uint8_t i = 0; i < 50; ++i) {
		block[i] = 'a';
	}

	uint64_t blockId;
	blockDevice.addBlock(&blockId);
	ASSERT_EQ(0, blockDevice.putBlock(blockId, block));
}

TEST_F(BlockDeviceFixture, readBlock) {
	uint8_t block[BLOCK_SIZE];
	memset(block, 0, BLOCK_SIZE);

	uint8_t readBlock[BLOCK_SIZE];
	memset(readBlock, 0, BLOCK_SIZE);

	for (int i = 0; i < 500; ++i) {
		block[i] = (uint8_t) to_string(i)[0];
	}

	uint64_t blockId;
	blockDevice.addBlock(&blockId);
	blockDevice.putBlock(blockId, block);

	ASSERT_EQ(0, blockDevice.getBlock(blockId, readBlock));
	ASSERT_TRUE(0 == memcmp(block, readBlock, BLOCK_SIZE));
}

TEST_F(BlockDeviceFixture, rootInode) {
	mtfs::inode_t inode;
	inode.accesRight = 0444;
	inode.uid = 1000;
	inode.gid = 1000;
	inode.size = 1024;
	inode.linkCount = 2;
	inode.atime = (uint64_t) time(NULL);
	inode.referenceId.clear();
	inode.dataBlocks.clear();

	mtfs::inode_t readInode;

	ASSERT_EQ(0, blockDevice.putInode(0, inode));
	ASSERT_EQ(0, blockDevice.getInode(0, readInode));
	ASSERT_EQ(inode, readInode);
}

TEST_F(BlockDeviceFixture, superblock) {
	mtfs::superblock_t superblock;
	superblock.iCacheSz = superblock.dCacheSz = superblock.bCacheSz = superblock.blockSz = 4096;
	superblock.migration = superblock.redundancy = 1;
	superblock.pools.clear();
	for (int i = 0; i < 5; ++i) {
		mtfs::pool_t pool;
		pool.migration = 0;
		pool.rule = NULL;
		pool.volumes.clear();
		superblock.pools[i] = pool;
	}


	ASSERT_TRUE(blockDevice.putSuperblock(superblock));
}

TEST_F(BlockDeviceFixture, putMetas) {
	mtfs::blockInfo_t blockInfo;
	blockInfo.referenceId.poolId = 1;
	blockInfo.referenceId.volumeId = 1;
	blockInfo.referenceId.id = 1;
	blockInfo.lastAccess = (uint64_t) time(NULL);

	ASSERT_TRUE(blockDevice.putBlockMetas(1, blockInfo));
}

TEST_F(BlockDeviceFixture, getMetas) {
	mtfs::blockInfo_t blockInfo, receiveInfo;
	memset(&blockInfo, 0, sizeof(mtfs::blockInfo_t));
	memset(&receiveInfo, 0, sizeof(mtfs::blockInfo_t));

	blockInfo.referenceId.poolId = 1;
	blockInfo.referenceId.volumeId = 1;
	blockInfo.referenceId.id = 2;
	blockInfo.lastAccess = (uint64_t) time(NULL);

	ASSERT_TRUE(blockDevice.putBlockMetas(2, blockInfo));
	ASSERT_TRUE(blockDevice.getBlockMetas(2, receiveInfo));
	ASSERT_EQ(blockInfo, receiveInfo);
}

TEST_F(BlockDeviceFixture, putDirBlock) {
	mtfs::ident_t id1, id2;
	id2.poolId = id1.poolId = 1;
	id1.volumeId = 1;
	id1.id = 2;
	id2.volumeId = 2;
	id2.id = 42;

	vector<mtfs::ident_t> ids;
	ids.push_back(id1);
	ids.push_back(id2);

	mtfs::dirBlock_t block = mtfs::dirBlock_t();
	block.entries.clear();
	block.entries.insert(make_pair("baz", ids));
	block.entries.insert(make_pair("test", ids));

	uint64_t dId = 0;

	ASSERT_EQ(0, blockDevice.addDirBlock(&dId));
	EXPECT_LE(0, dId);
	ASSERT_EQ(0, blockDevice.putDirBlock(dId, block));

	mtfs::dirBlock_t readBlock;
	ASSERT_EQ(0, blockDevice.getDirBlock(dId, readBlock));
	ASSERT_EQ(block.entries.size(), readBlock.entries.size());
	for (auto &&item: block.entries) {
		ASSERT_NE(readBlock.entries.end(), readBlock.entries.find(item.first));
		int i = 0;
		for (auto &&ident: item.second) {
			ASSERT_EQ(ident, readBlock.entries[item.first][i]);
			i++;
		}
	}

	ASSERT_EQ(0, blockDevice.delDirBlock(dId));
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}