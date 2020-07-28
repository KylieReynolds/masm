#include "../header/AgentUtil.h"
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

string AgentUtil::readDefinitionFile(string filePath)
{
	ostringstream oss;
	fstream file;
	try
	{
		file.open(filePath);
		string currentLine;
		while(getline(file, currentLine))
			oss << currentLine << '\n';
	}
	catch(exception ex)
	{
		cerr << ex.what();
	}

	/**  Replace new line with space  **/
	string fileString{oss.str()};
	for(int i{0}; i < fileString.length(); i++)
	{
		if(fileString[i] == '\n')
			fileString[i] = ' ';
	}

	return fileString;
};