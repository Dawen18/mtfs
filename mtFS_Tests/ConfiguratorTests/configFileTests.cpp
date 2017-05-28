/**
 * @author David Wittwer
 * @date 06.05.17.
**/

#include <gtest/gtest.h>
#include <rapidjson/document.h>
#include <mtfs/Mtfs.h>
#include <mtfs/TimeRule.h>
#include <mtfs/UserRightRule.h>

class ConfigFixture : public ::testing::Test {
public:
	ConfigFixture() {

	}

	virtual void SetUp() {
		d.SetObject();
	}

	virtual void TearDown() {
		d.RemoveAllMembers();
	}

	~ConfigFixture() {

	}

	rapidjson::Document d;
	rapidjson::Value v;
};

TEST_F(ConfigFixture, validateTime) {

	v.SetInt(2);
	rapidjson::Document::AllocatorType &allocator = d.GetAllocator();

	ASSERT_FALSE(mtfs::TimeRule::rulesAreValid(d));

	d.AddMember(rapidjson::StringRef("lowLimit"), v, allocator);
	ASSERT_TRUE(mtfs::TimeRule::rulesAreValid(d));

	d.AddMember(rapidjson::StringRef("highLimit"), v, allocator);
	ASSERT_TRUE(mtfs::TimeRule::rulesAreValid(d));

	d.RemoveMember("lowLimit");
	ASSERT_TRUE(mtfs::TimeRule::rulesAreValid(d));

	d.RemoveMember("highLimit");
	ASSERT_FALSE(mtfs::TimeRule::rulesAreValid(d));
}

TEST_F(ConfigFixture, validateRight) {
	ASSERT_FALSE(mtfs::UserRightRule::rulesAreValid(d));

	rapidjson::Document::AllocatorType &allocator = d.GetAllocator();
	v.SetInt(1);

	d.AddMember(rapidjson::StringRef("allowUsers"), v, allocator);
	ASSERT_TRUE(mtfs::UserRightRule::rulesAreValid(d));

	d.AddMember(rapidjson::StringRef("allowGroups"), v, allocator);
	ASSERT_TRUE(mtfs::UserRightRule::rulesAreValid(d));

	d.RemoveMember("allowUsers");
	ASSERT_TRUE(mtfs::UserRightRule::rulesAreValid(d));

	d.AddMember(rapidjson::StringRef("denyUsers"), v, allocator);
	ASSERT_TRUE(mtfs::UserRightRule::rulesAreValid(d));

	d.RemoveMember("allowGroups");
	ASSERT_TRUE(mtfs::UserRightRule::rulesAreValid(d));

	d.AddMember(rapidjson::StringRef("denyGroups"), v, allocator);
	ASSERT_TRUE(mtfs::UserRightRule::rulesAreValid(d));

	d.RemoveMember("denyUsers");
	ASSERT_TRUE(mtfs::UserRightRule::rulesAreValid(d));

	d.AddMember(rapidjson::StringRef("allowUsers"), v, allocator);
	d.AddMember(rapidjson::StringRef("allowGroups"), v, allocator);
	d.AddMember(rapidjson::StringRef("denyUsers"), v, allocator);
	ASSERT_TRUE(mtfs::UserRightRule::rulesAreValid(d));
}



int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}