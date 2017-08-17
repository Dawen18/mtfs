/**
 * \file Rule.h
 * \brief
 * \author David Wittwer
 * \version 0.0.1
 * \copyright GNU Publis License V3
 *
 * This file is part of MTFS.

    MTFS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FILESTORAGE_RULE_H
#define FILESTORAGE_RULE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>
#include <rapidjson/schema.h>
#include <mtfs/structs.h>
#include <mtfs/Volume.h>

/**
 *
 */
namespace mtfs {
	class Pool;

	class Volume;

	/**
	 * @interface
	 */
	class Rule {
	public:
		static constexpr const char *MIGRATION = "migration";    ///< Migration name in the config file

		static const int NO_MIGRATION = -1;    ///< No migration value
		static const int TIME_MIGRATION = 0;    ///< Time migration value
		static const int RIGHT_MIGRATION = 1;    ///< User right migration

		static const int SUCCESS = 0;    ///< Success code.
		static const int VALID_RULES = 0;    ///< Valid rule code.
		static const int INVALID_RULES = -1;    ///< Invalid rule code.
		static const int UNKNOW_MIGRATION = -2;    ///< Unknow migration code.
	public:

		/**
		 * @brief Copy JSON config.
		 *
		 * @param migration 	Migration type.
		 * @param source 		JSON source config.
		 * @param destination 	JSON destination config.
		 * @param allocator 	Allocator for destination.
		 *
		 * @return SUCCESS if no error or corresponding code.
		 */
		static int copyConfig(int migration, rapidjson::Document &source, rapidjson::Value &destination,
							  rapidjson::Document::AllocatorType &allocator);

		/**
		 * @brief Check if rule config is valid.
		 *
		 * @param migration Migration type.
		 * @param value Json config.
		 * @return VALID_RULES or INVALID_RULES
		 */
		static int rulesAreValid(int migration, const rapidjson::Value &value);

		/**
		 * @brief Build a Rule object.
		 *
		 * @param migration Migration type.
		 * @param value Json Config.
		 *
		 * @return Pointer on new Rule or nullptr if fail.
		 */
		static Rule *buildRule(int migration, const rapidjson::Value &value);

		/**
		 * Dump object to Json.
		 *
		 * @param json
		 * @param allocator
		 * @return
		 */
		virtual bool toJson(rapidjson::Value &json, rapidjson::Document::AllocatorType &allocator)=0;

		/**
		 * @brief Check if the block or inode satisfy rules.
		 * @param info Info of Block/Inode
		 *
		 * @return true or false.
		 */
		virtual bool satisfyRules(mtfs::ruleInfo_t info)=0;

		/**
		 * Configure volume storage
		 * @param volume
		 * @return
		 */
		virtual int configureStorage(Volume *volume);

		/**
		 * Configure pool storage
		 * @param volume
		 * @return
		 */
		virtual int configureStorage(Pool *pool);
	};

}  // namespace mtfs
#endif
