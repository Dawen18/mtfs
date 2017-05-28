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

TEST_F(BlockDeviceFixture, getInfosTest) {
	vector<string> infos = blockDevice.getInfos();
	vector<string> winfo;
	winfo.push_back("block");
	winfo.push_back("devicePath");
	winfo.push_back("fsType");

	ASSERT_EQ(infos.size(), winfo.size());

	for (auto ii = infos.begin(), wi = winfo.begin(); ii != infos.end(); ii++, wi++)
		EXPECT_EQ(*ii, *wi);

}

TEST_F(BlockDeviceFixture, addInode) {
	uint64_t inode = 0;
	ASSERT_TRUE(blockDevice.addInode(inode));
	ASSERT_NE(0, inode);

	uint64_t inode2 = 0;
	ASSERT_TRUE(blockDevice.addInode(inode2));
	ASSERT_NE(0, inode2);
	ASSERT_GT(inode2, inode);

	uint64_t inode3 = 0;
	ASSERT_TRUE(blockDevice.addInode(inode3));
}

TEST_F(BlockDeviceFixture, delInode) {
	uint64_t inode;
	blockDevice.addInode(inode);
	ASSERT_TRUE(blockDevice.delInode(inode));

	uint64_t inode2;
	blockDevice.addInode(inode2);
	blockDevice.addInode(inode2);
	blockDevice.addInode(inode2);
	blockDevice.delInode(inode);
}

TEST_F(BlockDeviceFixture, writeInode) {
	uint64_t inodeId;
	blockDevice.addInode(inodeId);

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
	ino.access = (unsigned long &&) time(nullptr);

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

	ASSERT_TRUE(blockDevice.writeInode(inodeId, ino));
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
	original.access = (unsigned long &&) time(nullptr);

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
	blockDevice.addInode(inodeId);
	cout << "inodeId " << inodeId << "  " << endl;
	blockDevice.writeInode(inodeId, original);
	ASSERT_TRUE(blockDevice.readInode(inodeId, inode));

//	cout << original.accesRight << " " << inode.accesRight << endl;
	ASSERT_EQ(original, inode);
	original.size = 2048;
	blockDevice.writeInode(inodeId, original);
	ASSERT_TRUE(blockDevice.readInode(inodeId, inode));
}

TEST_F(BlockDeviceFixture, addBlock) {
	uint64_t block = 0;
	ASSERT_TRUE(blockDevice.addBlock(block));
	ASSERT_NE(0, block);

	uint64_t block2 = 0;
	ASSERT_TRUE(blockDevice.addBlock(block2));
	ASSERT_NE(0, block2);
	ASSERT_GT(block2, block);

	uint64_t block3 = 0;
	ASSERT_TRUE(blockDevice.addBlock(block3));
}

TEST_F(BlockDeviceFixture, delBlock) {
	uint64_t block;
	blockDevice.addBlock(block);
	ASSERT_TRUE(blockDevice.delBlock(block));

	uint64_t block2;
	blockDevice.addBlock(block2);
	blockDevice.addBlock(block2);
	blockDevice.addBlock(block2);
	blockDevice.delBlock(block);
}

TEST_F(BlockDeviceFixture, writeBlock) {
	uint8_t block[BLOCK_SIZE];
	memset(block, 0, BLOCK_SIZE);
	for (uint8_t i = 0; i < 50; ++i) {
		block[i] = 'a';
	}

	uint64_t blockId;
	blockDevice.addBlock(blockId);
	ASSERT_TRUE(blockDevice.writeBlock(blockId, block));
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
	blockDevice.addBlock(blockId);
	blockDevice.writeBlock(blockId, block);

	ASSERT_TRUE(blockDevice.readBlock(blockId, readBlock));
	ASSERT_TRUE(0 == memcmp(block, readBlock, BLOCK_SIZE));
}

TEST_F(BlockDeviceFixture, rootInode) {
	mtfs::inode_t inode;
	inode.accesRight = 0444;
	inode.uid = 1000;
	inode.gid = 1000;
	inode.size = 1024;
	inode.linkCount = 2;
	inode.access = (uint64_t) time(NULL);
	inode.referenceId.clear();
	inode.dataBlocks.clear();

	mtfs::inode_t readInode;

	ASSERT_TRUE(blockDevice.writeInode(0, inode));
	ASSERT_TRUE(blockDevice.readInode(0, readInode));
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


	ASSERT_TRUE(blockDevice.writeSuperblock(superblock));
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}