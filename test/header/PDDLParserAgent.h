#ifndef PDDLPARSERAGENT_H
#define PDDLPARSERAGENT_H

#include "ParserAgentDefinition.h"
#include "PDDLParserDomain.h"
#include "Tokens.h"
#include <deque>

enum class ConType
{
	PRECONDITION,
	POSTCONDITION
};

class ConditionType
{
public:
	static std::string toStr(ConType ct);
	static std::string name;
	ConditionType(std::string s) { name = s; };
};

enum class ParType
{
	INPUT,
	OUTPUT,
	PARAMETER
};

class ParameterType
{
public:
	static std::string toStr(ParType pt);
	static std::string name;
	ParameterType(std::string s) { name = s; };
};

class PDDLParserAgent : public Tokens
{
private:
	std::map<int, ParserAgentDefinition> agents;
	Tokens tokenParser;
	PDDLParserDomain domainParser;

public:
	static const int misformattedString = -1;	// Agent definition is incorrectly formatted

	PDDLParserAgent() {};	// Default constructor
	PDDLParserAgent(Tokens aTokenParser, PDDLParserDomain aDomainParser);

	ParserAgentDefinition getAgent(int agentId);
	void printAgents();

private:
	bool parseAgentDomains(ParserAgentDefinition agentDef);
	bool parseEffects(ParserAgentDefinition agentDef);
	bool parseIO(ParserAgentDefinition agentDef, ParType parType);
	bool parseExpression(std::deque<int> conditions, ConType conType, ParserAgentDefinition agentDef);
	bool parseConditions(ParserAgentDefinition agentDef, ConType conType);

public:
	int parseAgent();
};

#endif