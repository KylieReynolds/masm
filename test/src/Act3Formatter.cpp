// Formatter for logger

#include "../header/Act3Formatter.h"

#include <sstream>
#include <time.h>
using namespace std;

string Act3Formatter::getTime()
{
	mtx.lock();
	ostringstream oss;

	time_t rawtime;
	struct tm* timeinfo;
	char timeBuf[50];

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(timeBuf, 50, "%T", timeinfo);
	oss << timeBuf;

	mtx.unlock();

	return oss.str();
};

string Act3Formatter::getMessage(string level, string name, string messageText)
{
	/**   Log message should be: level time [name]: message   ***/
	mtx.lock();

	string message = level + " " + getTime() + " [" + name + "]: " + messageText + '\n';
	return message;

	mtx.unlock();
};