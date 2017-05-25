#ifndef PLUGINSYSTEM_PLUGIN_H
#define PLUGINSYSTEM_PLUGIN_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include <mtfs/structs.h>
#include <map>

namespace pluginSystem {
	typedef struct {
		std::string name;
		std::vector<std::string> params;
	} pluginInfo_t;

	class Plugin {
	public:
		static constexpr const char *TYPE = "type";


		virtual std::vector<std::string> getInfos()=0;

		virtual bool attach(std::map<std::string, std::string> params)=0;

		virtual bool detach()=0;

		virtual bool addInode(std::uint64_t &inodeId)=0;

		virtual bool delInode(std::uint64_t inodeId)=0;

		virtual bool readInode(std::uint64_t inodeId, mtfs::inode_st &inode)=0;

		virtual bool writeInode(std::uint64_t inodeId, mtfs::inode_st &inode)=0;

		virtual bool addBlock(std::uint64_t &blockId)=0;

		virtual bool delBlock(std::uint64_t blockId)=0;

		virtual bool readBlock(std::uint64_t blockId, std::uint8_t *buffer)=0;

		virtual bool writeBlock(std::uint64_t blockId, std::uint8_t *buffe)=0;

		virtual bool readSuperblock(mtfs::superblock_t &superblock)=0;

		virtual bool writeSuperblock(mtfs::superblock_t &superblock)=0;

	};

}  // namespace Plugin
#endif
