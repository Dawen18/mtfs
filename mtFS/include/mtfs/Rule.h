/**
 * @file Rule.h
 * @brief Interface for rules objects
 * @author David Wittwer
 * @version 0.0.1
 * @date 01.05.2017
 * @copyright 2017
 */

#ifndef FILESTORAGE_RULE_H
#define FILESTORAGE_RULE_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <assert.h>
#include <mtfs/structs.h>
#include <rapidjson/schema.h>

/**
 *
 */
namespace mtfs {

	typedef struct ruleInfo_st {
		uint16_t uid;

		uint16_t gid;

		uint64_t previousAccess;

	} ruleInfo_t;

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
		 * @brief Check if the block or inode satisfy rules.
		 * @param info Info of Block/Inode
		 *
		 * @return true or false.
		 */
		virtual bool satisfyRules(ruleInfo_st info)=0;

		virtual bool toJson(rapidjson::Value &json, rapidjson::Document::AllocatorType &allocator)=0;
	};

}  // namespace mtfs
#endif
