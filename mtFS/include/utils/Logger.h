/// \file Logger.h
/// \brief
/// \author David Wittwer
/// \version 0.0.1
/// \date 16.07.17

#ifndef MTFS_LOGGER_H
#define MTFS_LOGGER_H

#include <string>
#include <ostream>

class Logger {
public:
	enum level {
		L_ERROR,
		L_WARNING,
		L_INFO,
		L_DEBUG,
	};

	static void init(std::ostream &logStream, const level dLevel = L_ERROR);

	static Logger *getInstance();

	void log(const std::string &key, const std::string &message, const level &level);

private:
	static Logger *instance;

	const level displayLevel;
	std::ostream &outStream;

	Logger(const level dLevel, std::ostream &logStream);

	std::string levelString(const level &l);
};


#endif //MTFS_LOGGER_H
