/**
 * @author David Wittwer
 * @date 22.04.17.
**/

#include <gtest/gtest.h>
#include <Plugin/BlockDevice/BlockDevice.h>

TEST(test1, te){
	PluginSystem::BlockDevice blockDevice();
	ASSERT_EQ(1,1);
}

int main(int argc, char** argv){
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}