/**
 * \file Logger.cpp
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

#include <iostream>
#include "utils/Logger.h"

using namespace std;

Logger *Logger::instance = nullptr;

void Logger::init(std::ostream &logStream, const level dLevel) {
	instance = new Logger(dLevel, logStream);
}

Logger *Logger::getInstance() {
	if (nullptr == instance)
		instance = new Logger(level::L_ERROR, cerr);

	return instance;
}

Logger::Logger(const level dLevel, ostream &logStream) : displayLevel(dLevel), outStream(logStream) {}

void Logger::log(const std::string &key, const std::string &message, const level &level) {
	if (this->displayLevel >= level) {
		this->outStream << "MtFS [" << this->levelString(level) << "] " << key << ": " << message << endl;
		this->outStream.flush();
	}
}

std::string Logger::levelString(const Logger::level &l) {
	switch (l) {
		case L_INFO:
			return "INFO";
		case L_WARNING:
			return "WARNING";
		case L_ERROR:
			return "ERROR";
		case L_DEBUG:
			return "DEBUG";
		default:
			return "UNKNOW";
	}
}
