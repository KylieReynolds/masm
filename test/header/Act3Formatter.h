// Formatter for logger

#ifndef ACT3FORMATTER_H
#define ACT3FORMATTER_H

#include <string>

#include <poco/Mutex.h>

/**   Log should be: level time [name]: message   ***/
class Act3Formatter
{
private:
	Poco::Mutex mtx;

public:
	Act3Formatter() {};

	std::string getTime();
	std::string getMessage(std::string level, std::string name, std::string messageText);
};

#endif