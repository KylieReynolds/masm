#ifndef PDDLPARSER_H
#define PDDLPARSER_H

#include "ParserAgentDefinition.h"
#include "PDDLParserAgent.h"
#include "PDDLParserDomain.h"
#include "Tokens.h"

class PDDLParser : public Tokens
{
private:
	Tokens tokenParser;
	PDDLParserAgent agentParser;
	PDDLParserDomain domainParser;

public:
	PDDLParser() {};

	Tokens getTokenParser() { return tokenParser; };
	PDDLParserAgent getParserAgent() { return agentParser; };
	PDDLParserDomain getParserDomain() { return domainParser; };

	void printDefinitions();
	void printDefinition(int id);

private:
	int parseDefine();
	std::list<int> parseFile(std::string inputFile);

public:
	std::list<int> parseString(std::string inputString);
	std::list<int> parseDirectory(std::string dirPath);
};

#endif
