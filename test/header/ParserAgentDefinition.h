#ifndef PARSERAGENTDEFINITION_H
#define PARSERAGENTDEFINITION_H

#include "AgentCompatibility.h"
#include "PDDLParserDomain.h"
#include "Tokens.h"
#include <deque>
#include <map>

class ExpressionList;

class ParserAgentDefinition : public Tokens
{
private:
	ExpressionList* expressionList;

	std::string name;
	int id;
	std::vector<int> domains;
	std::map<int, int> parameters;
	std::map<int, int> inputs;
	std::map<int, int> outputs;

	// Using deque instead of stack because they have the same functionality but deque adds iteration capabilities
	std::deque<int> preconditions;	// must all be satisfied for agent to act
	std::deque<int> postconditions;	// will occur when agent is finished acting

	Tokens tokenParser;
	PDDLParserDomain domainParser;
	std::vector<ExpressionList> preconditionsMap;
	std::string definition;

public:
	ParserAgentDefinition() {};	// Default constructor
	ParserAgentDefinition(std::string aName, int anId, Tokens aTokenParser, PDDLParserDomain aDomainParser);
	ParserAgentDefinition(const ParserAgentDefinition& pad);	// copy constructor

	std::map<int, int> getParameters() { return parameters; };
	std::map<int, int> getInputs() { return inputs; };
	std::map<int, int> getOutputs() { return outputs; };
	std::string getDefinition() { return definition; };

	void addDomain(int domainId) { domains.emplace_back(domainId); };
	void addParameter(int variable, int domain) { parameters.emplace(variable, domain); };
	void addInput(int variable, int domain) { inputs.emplace(variable, domain); };
	void addOutput(int variable, int domain) { outputs.emplace(variable, domain); };

	void setPreconditions(std::deque<int> conditions) { preconditions = conditions; };
	void setPostconditions(std::deque<int> conditions) { postconditions = conditions; };

	std::string toString();	// print associated agent definition 
	std::string swapVariableDomain(std::string fullname);	// Swap the variable name with its domain name
	ExpressionList getPredicateSet(std::map<int, int> io);
	int getType(int token);

private:
	Expression getExpression(int field1, std::string field1_name, std::string field1_domain,
		int field2, std::string field2_name, std::string field2_domain, int comparison);
	ExpressionList getExpressionList(std::vector<int> fieldList, int domainVariable);
	std::vector<ExpressionList> buildList(std::deque<int> conditions, std::map<int, int> io);

public:
	std::vector<ExpressionList> getPreconditions();
	AgentCompatibility checkAgentCompatibility(ParserAgentDefinition subscriber);
};

class TwoDimExpressionList
{
public:
	std::vector<ExpressionList> conditionList;
	bool boolValue;
};

#endif