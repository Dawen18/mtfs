/**
 * @author David Wittwer
 * @date 22.04.17.
**/

#include <gtest/gtest.h>
#include <Plugin/BlockDevice/BlockDevice.h>

using namespace std;
using namespace PluginSystem;

const string HOME = "/tmp/mtFS/Plugins/BlockDevices/";

TEST(BlockDevice, attachDetach
) {
#ifndef DEBUG
	if (setuid(0) != 0)
		cout << "fail setuid" << endl;
#endif

	BlockDevice blockDevice;
	map<string, string> params;
	params.
			insert(make_pair("home", HOME));
	params.
			insert(make_pair("blocksize", "4096"));
	params.
			insert(make_pair("devicePath", "/dev/sdd1"));
	params.
			insert(make_pair("fsType", "ext4"));
	ASSERT_TRUE(blockDevice
						.
								attach(params)
	);
	ASSERT_TRUE(blockDevice
						.

								detach()

	);
}

class BlockDeviceFixture : public ::testing::Test {
public:
	BlockDeviceFixture() {
	}

	virtual void SetUp() {
		map<string, string> params;
		params.insert(make_pair("home", HOME));
		params.insert(make_pair("blocksize", "4096"));
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

};

TEST_F(BlockDeviceFixture, getInfosTest) {
	vector<string> infos = blockDevice.getInfos();
	vector<string> winfo;
	winfo.push_back("block");
	winfo.push_back("devicePath");

	ASSERT_EQ(infos.size(), winfo.size());

	for (auto ii = infos.begin(), wi = winfo.begin(); ii != infos.end(); ii++, wi++)
		EXPECT_EQ(*ii, *wi);

}

TEST_F(BlockDeviceFixture, addInode
) {
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

TEST_F(BlockDeviceFixture, delInode
) {
	uint64_t inode;
	blockDevice.addInode(inode);
	ASSERT_TRUE(blockDevice.delInode(inode));

	uint64_t inode2;
	blockDevice.addInode(inode2);
	blockDevice.addInode(inode2);
	blockDevice.addInode(inode2);
	blockDevice.delInode(inode);
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}