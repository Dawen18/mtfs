#ifndef FILESTORAGE_DIRECTORY_ENTRY_ACCESS_H
#define FILESTORAGE_DIRECTORY_ENTRY_ACCESS_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>

#include <FileStorage/structs.h>

namespace FileStorage {
	class DirectoryEntryAccess {
	public:
		virtual bool addEntry(inode_st &parentInode, std::string entry, inode_st &entryInode)=0;

		virtual bool delEntry(inode_st &parentInode, std::string entry)=0;

		virtual bool getEntry(inode_st &parentInode, std::string entry, inode_st &entryInode)=0;

		virtual bool setEntry(inode_st &parentInode, std::string entry, inode_st &entryInode)=0;

		virtual bool addLink(inode_st &parentInode, std::string link, ident_st linkId)=0;

		virtual bool delLink(inode_st &parentInode, std::string link)=0;

		virtual bool getRoot(inode_st &rootInode)=0;

	};

}  // namespace FileStorage
#endif
