#include <mtfs/Cache.h>
#include <mtfs/Mtfs.h>
#include <mtfs/TimeRule.h>
#include <Configurator/ConfigFile.h>
#include <mtfs/UserRightRule.h>
#include <pwd.h>

namespace mtfs {
	using namespace std;
	Mtfs *Mtfs::instance = 0;
	thread *Mtfs::thr = 0;

	Mtfs::Mtfs() {
//		PoolManager *poolManager = new PoolManager();
//		inodes = poolManager;
//		dirEntry = poolManager;
//		blocs = poolManager;
	}

	void Mtfs::loop() {

		Mtfs *mtfs = getInstance();
		int loc = 0;
		std::cout << "thread\n";
		for (int i = 0; i < 10; ++i) {
			std::this_thread::sleep_for(std::chrono::milliseconds(i));
			loc++;
			std::cout << loc << "\n";
		}

	}

	Mtfs *Mtfs::getInstance() {
		if (!instance)
			instance = new Mtfs();

		return instance;
	}

	void Mtfs::start() {
		thr = new std::thread(&loop);
	}

	void Mtfs::join() {
		thr->join();
	}

	bool Mtfs::migrationIsValid(const int migration) {
		return (migration == TIME_MIGRATION || migration == RIGHT_MIGRATION);
	}

	Rule *Mtfs::buildRule(int migration, rapidjson::Value &value) {
		Rule *rule = nullptr;
		cout << "value type " << value.GetType() << endl;
		if (migration == TIME_MIGRATION) {
//			uint64_t ll = 0, hl = 0;
//
//			if (value.HasMember(Configurator::ConfigFile::TIME_LOW_LIMIT))
//				ll = value[Configurator::ConfigFile::TIME_LOW_LIMIT].GetUint64();
//
//			if (value.HasMember(Configurator::ConfigFile::TIME_HIGH_LIMIT))
//				hl = value[Configurator::ConfigFile::TIME_HIGH_LIMIT].GetUint64();
//
//			rule = new TimeRule(ll, hl);
		} else if (migration == RIGHT_MIGRATION) {
			UserRightRule *uRule = new UserRightRule();

//			if (value.HasMember(Configurator::ConfigFile::ALLOW_USER))
//				for (auto &ua: value[Configurator::ConfigFile::ALLOW_USER].GetArray())
//					uRule->addAllowUid(getpwnam(ua.GetString())->pw_uid);
//
//			if (value.HasMember(Configurator::ConfigFile::DENY_USER))
//				for (auto &ud: value[Configurator::ConfigFile::DENY_USER].GetArray())
//					uRule->addDenyUid(getpwnam(ud.GetString())->pw_uid);
//
//			if (value.HasMember(Configurator::ConfigFile::ALLOW_GROUP))
//				for (auto &ga: value[Configurator::ConfigFile::ALLOW_GROUP].GetArray())
//					uRule->addAllowGid(getpwnam(ga.GetString())->pw_gid);
//
//			if (value.HasMember(Configurator::ConfigFile::DENY_GROUP))
//				for (auto &ga: value[Configurator::ConfigFile::DENY_GROUP].GetArray())
//					uRule->addAllowGid(getpwnam(ga.GetString())->pw_gid);

			rule = uRule;
		}
		return rule;
	}


}  // namespace mtfs
