/// \file Logger.cpp
/// \brief
/// \author David Wittwer
/// \version 0.0.1
/// \date 16.07.17

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
