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
		static constexpr const char *TYPE = "plName";
		static constexpr const char *PARAMS = "params";

		virtual std::string getName()=0;

		virtual bool attach(std::map<std::string, std::string> params)=0;

		virtual bool detach()=0;

		virtual int addInode(uint64_t *inodeId)=0;

		virtual int delInode(const uint64_t &inodeId)=0;

		virtual int getInode(const uint64_t &inodeId, mtfs::inode_st &inode)=0;

		virtual int putInode(const uint64_t &inodeId, const mtfs::inode_st &inode)=0;

		virtual int getInodeMetas(const uint64_t &inodeId, mtfs::blockInfo_t &metas)=0;

		virtual int putInodeMetas(const uint64_t &inodeId, const mtfs::blockInfo_t &metas)=0;

		virtual int addDirBlock(uint64_t *id)=0;

		virtual int delDirBlock(const uint64_t &id)=0;

		virtual int getDirBlock(const uint64_t &id, mtfs::dirBlock_t &block)=0;

		virtual int putDirBlock(const uint64_t &id, const mtfs::dirBlock_t &block)=0;

		virtual int getDirBlockMetas(const uint64_t &id, mtfs::blockInfo_t &metas)=0;

		virtual int putDirBlockMetas(const uint64_t &id, const mtfs::blockInfo_t &metas)=0;

		virtual int addBlock(uint64_t *blockId)=0;

		virtual int delBlock(const uint64_t &blockId)=0;

		virtual int getBlock(const uint64_t &blockId, std::uint8_t *buffer)=0;

		virtual int putBlock(const uint64_t &blockId, const uint8_t *buffer)=0;

		virtual int getBlockMetas(const uint64_t &blockId, mtfs::blockInfo_t &metas)=0;

		virtual int putBlockMetas(const uint64_t &blockId, const mtfs::blockInfo_t &metas)=0;

		virtual bool getSuperblock(mtfs::superblock_t &superblock)=0;

		virtual bool putSuperblock(const mtfs::superblock_t &superblock)=0;

	};

}  // namespace Plugin
#endif
