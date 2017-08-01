#ifndef PLUGINSYSTEM_PLUGIN_H
#define PLUGINSYSTEM_PLUGIN_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <cassert>

#include <mtfs/structs.h>
#include <map>

namespace pluginSystem {
	typedef struct {
		std::string name;
		std::vector<std::string> params;
	} pluginInfo_t;

	class Plugin {
	public:
		static constexpr const char *TYPE = "plName";
		static constexpr const char *PARAMS = "params";

		virtual std::string getName()=0;

		virtual bool attach(std::map<std::string, std::string> params)=0;

		virtual bool detach()=0;

		virtual int add(uint64_t *id, const mtfs::blockType &type)=0;

		virtual int del(const uint64_t &id, const mtfs::blockType &type)=0;

		virtual int get(const uint64_t &id, void *data, const mtfs::blockType &type, bool metas)=0;

		virtual int put(const uint64_t &id, const void *data, const mtfs::blockType &type, bool metas)=0;

		virtual bool getSuperblock(mtfs::superblock_t &superblock)=0;

		virtual bool putSuperblock(const mtfs::superblock_t &superblock)=0;

	};

}  // namespace Plugin
#endif
