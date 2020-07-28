#include "..\header\PDDLParser.h"
#include "poco/DirectoryIterator.h"

using namespace std;

void PDDLParser::printDefinitions()
{
	// Print all definitions
	domainParser.printDomains();
	agentParser.printAgents();
};

void PDDLParser::printDefinition(int id)
{
	// Print definition for specific agent or domain
	// null checking is not possible but unneeded because of error circumventions in called methods
	domainParser.printDomain(id);
	ParserAgentDefinition agent = agentParser.getAgent(id);
	clog << "INFO: " << agent.toString();
};

int PDDLParser::parseDefine()
{
	/**********
	* Methods for providing and parsing agent definition
	*   --- currently the agent definition can be passed via string or file
	*   --- if passed via file, the "file" can either be an individual file or a directory.  If it's a directory, 
	*		the parser will parse all files in the directory and recurse through subdirectories
	**********/

	int id = -1;
	if (getNextToken() != OpenParen)
	{
		cerr << "WARNING: PDDLParser - definition parsing -- missing open parenthesis -- returning";
		return id;
	}

	switch (getNextToken())
	{
	case Agent:
		id = agentParser.parseAgent();
		break;
	case Domain:
		id = domainParser.parseDomain();
	default:
		cerr << "WARNING: PDDLParser - definition parsing -- missing valid definition type -- returning";
		break;
	}
	return id;
};

list<int> PDDLParser::parseFile(string inputFile)
{
	list<int> retList;
	int retId;
	bool atleastOne = false;

	try
	{
		ifstream ifs(inputFile);
		if (ifs.is_open())
			setInputStream(ifs);
		else
			cerr << "ERROR: No file opened";

		// First two tokens of each input file are "(" and "define"
		setLookahead(-2);
		while (true)
		{
			if (getNextToken() != OpenParen || getNextToken() != Define || getNextToken() != CloseParen)
			{
				ifs.close();
				return retList;
			}
			retId = parseDefine();
			if (retId == -1)
			{
				ifs.close();
				return retList;
			}
			else
				retList.push_back(retId);
			atleastOne = true;
		}
	}
	catch (ifstream::failure e)
	{
		cerr << "ERROR: Caught ifstream exception: " << e.what();
		return retList;
	}
};

list<int> PDDLParser::parseString(string inputString)
{
	// Provides the tokens as a list
	list<int> ids;
	if (inputString.empty())
		return ids;

	setCurrentPos(0);
	setInputString(inputString);
	setLookahead(-2);
	while (getNextToken() == OpenParen)	// would this really happen many times in a row?
	{
		if (getNextToken() != Define)
			return ids;
		int id = parseDefine();
		if (id != -1)
			ids.push_back(id);
	}
	return ids;
};

list<int> PDDLParser::parseDirectory(string dirPath)
{
	list<int> ids;
	Poco::DirectoryIterator end;
	for (Poco::DirectoryIterator it(dirPath); it != end; ++it)
	{
		clog << "INFO: PDDLParser - Processing {" << it.name() << "}";
		if (it->isDirectory())
			parseDirectory(it->path());
		else if (it->isFile())
			ids.emplace_back(parseFile(it->path()));
		else
			cerr << "ERROR: Incorrect directory listing";
	}
	return ids;
};