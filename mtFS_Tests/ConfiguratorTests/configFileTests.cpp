/**
 * @author David Wittwer
 * @date 06.05.17.
**/

#include <gtest/gtest.h>
#include <Configurator/ConfigFile.h>
#include <rapidjson/document.h>

class ConfigFileFixture : public ::testing::Test {
public:
	ConfigFileFixture() {

	}

	virtual void SetUp() {
		d.SetObject();
		configFile = new Configurator::ConfigFile(d);
	}

	virtual void TearDown() {
		d.RemoveAllMembers();
	}

	~ConfigFileFixture() {

	}

	rapidjson::Document d;
	rapidjson::Value v;
	Configurator::ConfigFile *configFile;
};

TEST_F(ConfigFileFixture, validateTime) {

	v.SetInt(2);
	rapidjson::Document::AllocatorType &allocator = d.GetAllocator();

	ASSERT_FALSE(configFile->validateTime(d));

	d.AddMember(rapidjson::StringRef("lowLimit"), v, allocator);
	ASSERT_TRUE(configFile->validateTime(d));

	d.AddMember(rapidjson::StringRef("highLimit"), v, allocator);
	ASSERT_TRUE(configFile->validateTime(d));

	d.RemoveMember("lowLimit");
	ASSERT_TRUE(configFile->validateTime(d));

	d.RemoveMember("highLimit");
	ASSERT_FALSE(configFile->validateTime(d));
}

TEST_F(ConfigFileFixture, validateRight) {
	ASSERT_FALSE(configFile->validateRight(d));

	rapidjson::Document::AllocatorType &allocator = d.GetAllocator();
	v.SetInt(1);

	d.AddMember(rapidjson::StringRef("allowUsers"), v, allocator);
	ASSERT_TRUE(configFile->validateRight(d));

	d.AddMember(rapidjson::StringRef("allowGroups"), v, allocator);
	ASSERT_TRUE(configFile->validateRight(d));

	d.RemoveMember("allowUsers");
	ASSERT_TRUE(configFile->validateRight(d));

	d.AddMember(rapidjson::StringRef("denyUsers"), v, allocator);
	ASSERT_TRUE(configFile->validateRight(d));

	d.RemoveMember("allowGroups");
	ASSERT_TRUE(configFile->validateRight(d));

	d.AddMember(rapidjson::StringRef("denyGroups"), v, allocator);
	ASSERT_TRUE(configFile->validateRight(d));

	d.RemoveMember("denyUsers");
	ASSERT_TRUE(configFile->validateRight(d));

	d.AddMember(rapidjson::StringRef("allowUsers"), v, allocator);
	d.AddMember(rapidjson::StringRef("allowGroups"), v, allocator);
	d.AddMember(rapidjson::StringRef("denyUsers"), v, allocator);
	ASSERT_TRUE(configFile->validateRight(d));
}



int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}