#include "..\header\PDDLParserAgent.h"

using namespace std;

// Make enums match up to string values
string ConditionType::toStr(ConType ct)
{
	switch (ct)
	{
	case ConType::PRECONDITION:
		return "precondition";
	case ConType::POSTCONDITION:
		return "postcondition";
	default:
		return "";
	}
};
string ParameterType::toStr(ParType pt)
{
	switch (pt)
	{
	case ParType::INPUT:
		return "input";
	case ParType::OUTPUT:
		return "output";
	case ParType::PARAMETER:
		return "parameter";
	default: 
		return "";
	}
};

PDDLParserAgent::PDDLParserAgent(Tokens aTokenParser, PDDLParserDomain aDomainParser)
{
	tokenParser = aTokenParser;
	domainParser = aDomainParser;
};

ParserAgentDefinition PDDLParserAgent::getAgent(int agentId)
{
	if (agents.find(agentId) != agents.end())
		return agents.at(agentId);
	else
		return {};	// return null
};

void PDDLParserAgent::printAgents()
{
	map<int, ParserAgentDefinition>::iterator it;
	for(it = agents.begin(); it != agents.end(); it++)
	{
		ParserAgentDefinition agentDef = it->second;
		clog << "INFO: " << agentDef.toString();
	}
};

bool PDDLParserAgent::parseAgentDomains(ParserAgentDefinition agentDef)
{	
	// Parse the domain section of the agent definition
	bool atleastOne = false;
	
	while (true)	// Iterate through the list of domains and add each one to the agent definition
	{
		string domainName = getNextWord();
		if (domainName.size() == 0)
		{
			cerr << "WARNING: PDDLParserAgent -- agent parsing -- domain name is blank -- returning";
			return false;
		}
		
		int domainId = tokenize(domainName);
		if (domainId == CloseParen)
		{
			if (atleastOne) 
				break;
			cerr << "WARNING: PDDLParserAgent -- agent parsing -- no domains defined -- returning";
			return false;
		}
		else if (isPredefined(domainId))
		{
			cerr << "WARNING: PDDLParserAgent -- agent parsing -- domain name " << domainId << " is a reserved word {" << getVariableName(domainId) << "} -- returning";
			return false;
		}
		else if (domainParser.getDomain(domainId).name.empty()) // null domain definition
		{
			cerr << "WARNING: PDDLParserAgent -- agent parsing -- domain name " << domainId << " (" << getVariableName(domainId) << ") must be defined prior to using it in an agent -- returning";
			return false;
		}

		atleastOne = true;
		agentDef.addDomain(domainId);
	}

	return true;
};

bool PDDLParserAgent::parseEffects(ParserAgentDefinition agentDef)
{
	// Effects are not a thing so this is a useless shell
	if (getNextToken() != OpenParen || getNextToken() != CloseParen)
	{
		cerr << "WARNING: PDDLParserAgent -- effect parsing -- effect isn't blank -- returning";
		return false;
	}
	getNextToken();
	return true;
};

bool PDDLParserAgent::parseIO(ParserAgentDefinition agentDef, ParType parType)
{
	// Parse input and output definitions of agent definition
	bool atleastOneIO = false;
	bool isFirst = true;
	list<int> ioList;

	if (getNextToken() != OpenParen)
		return false;

	while (true)	// Iterate through input/output definition
	{
		string paramType = ParameterType::toStr(parType);
		ioList.clear();
		bool atleastOne = false;

		while (true)
		{	// definitions have format "x y z - type"
			string ioName = getNextWord();
			if (ioName.empty())
			{
				cerr << "WARNING: PDDLParserAgent -- " << paramType << " parsing -- " << paramType << " name is blank -- returning";
				return false;
			}

			int ioId = tokenize(ioName);
			if (ioId == Dash)	// dash indicates all names are collected
			{
				if (atleastOne)
					break;
				cerr << "WARNING: PDDLParserAgent -- " << paramType << " parsing -- no " << paramType << " names -- returning";
				return false;
			}
			else if (ioId == Null)
			{
				if (isFirst)
				{
					if (getNextToken() != CloseParen)
						return false;
					getNextToken();
					return true;
				}
				cerr << "WARNING: PDDLParserAgent -- " << paramType << " parsing -- NULL encountered -- returning";
				return false;
			}
			else if (ioId == CloseParen)
			{
				if (atleastOneIO && !atleastOne)
				{
					getNextToken();
					return true;
				}
				cerr << "WARNING: PDDLParserAgent -- " << paramType << " parsing -- no " << paramType << " -- returning";
				return false;
			}

			if (isPredefined(ioId))
			{
				cerr << "WARNING: PDDLParserAgent -- " << paramType << " parsing -- " << paramType << " name " << ioName << " is a reserved word -- returning";
				return false;
			}

			isFirst = false;
			atleastOne = true;
			ioList.push_back(ioId);
		}

		string domainName = getNextWord();
		if (domainName.empty())
		{
			cerr << "WARNING: PDDLParserAgent -- " << paramType << " parsing -- input name is blank -- returning";
			return false;
		}

		int domainId = tokenize(domainName);
		if (isPredefined(domainId))
		{
			cerr << "WARNING: PDDLParserAgent -- " << paramType << " parsing -- domain name " << domainName << " is a reserved word -- returning";
			return false;
		}
		else if (domainParser.getDomain(domainId).name.empty()) // null domain definition
		{
			cerr << "WARNING: PDDLParserAgent -- " << paramType << " parsing -- domain name " << domainId << " (" << getVariableName(domainId) << ") must be defined prior to using it in an agent -- returning";
			return false;
		}

		for (int variableId : ioList)
		{
			switch (parType)
			{
			case ParType::INPUT:
				agentDef.addInput(variableId, domainId);
				break;
			case ParType::OUTPUT:
				agentDef.addOutput(variableId, domainId);
				break;
			case ParType::PARAMETER:
				agentDef.addParameter(variableId, domainId);
				break;
			default:
				cerr << "WARNING: PDDLParserAgent -- " << paramType << " parsing -- " << paramType << " is neither input, output, or parameter";
			}
		}
		atleastOneIO = true;
	}
};

bool PDDLParserAgent::parseExpression(deque<int> conditions, ConType conType, ParserAgentDefinition agentDef)
{
	// Helper method to parse conditions section of agent definition
	bool atleastOne = false;
	int token = nextToke();
	int count;
	switch (token)
	{
	case START:
		getNextToken();
		return true;

	case OpenParen:
		getNextToken();
		if (!parseExpression(conditions, conType, agentDef))
			return false;
		else if (nextToke() == CloseParen)
		{
			getNextToken();
			return true;
		}
		cerr << "WARNING: PDDLParserAgent -- " << ConditionType::toStr(conType) << " parsing -- unbalanced parentheses -- returning";
		return false;

	case And:
	case Or:	// Assuming there are at least two expressions
		count = 0;
		getNextToken();
		while (true)
		{
			if (!parseExpression(conditions, conType, agentDef))
				return false;
			count++;	// number of expressions
			if (count >= 2)
			{
				conditions.push_back(token);
				if (nextToke() == CloseParen)
					return true;
			}
			else if (count == 1 && nextToke() == CloseParen)
			{
				conditions.push_back(True);
				conditions.push_back(token);
				if (nextToke() == CloseParen)
					return true;
			}
		}

	case Decrease:
	case Increase:
	case Assign:
	case Equal:
	case LessThan:
	case GreaterThan:
	case NE:
	case GE:
	case LE:
		getNextToken();
		if (!parseExpression(conditions, conType, agentDef))
			return false;	// get 1st expression or return false
		if (!parseExpression(conditions, conType, agentDef))
			return false;	// get 2nd expression or return false
		conditions.push_back(token);
		return true;

	default:
		if (agentDef.getInputs().find(nextToke()) != agentDef.getInputs().end() ||
			agentDef.getOutputs().find(nextToke()) != agentDef.getOutputs().end() ||
			agentDef.getParameters().find(nextToke()) != agentDef.getParameters().end())	// if any of these exist
		{
			string variablename = getVariableName(nextToke());
			int peek = getNextToken();
			if (peek == VariableSeparator)
			{
				variablename += "::" + getNextWord();
				peek = getNextToken();
				if (peek == VariableSeparator)
				{
					variablename += "::" + getNextWord();
					peek = getNextToken();
				}
			}
			int variableId = tokenize(variablename);
			conditions.push_back(variableId);
			return true;
		}
		else if (!isNumber(nextToke()) && isVariable(nextToke()))
		{
			if (!domainParser.isPredicate(token) && !domainParser.isFunction(token))
			{
				conditions.push_back(token);
				getNextToken();
				return true;
			}
			getNextToken();
			while (nextToke() != CloseParen)
				if (!parseExpression(conditions, conType, agentDef))
					return false;
			conditions.push_back(token);
			return true;
		}
		else
		{
			conditions.push_back(nextToke());
			getNextToken();
		}
		return true;
	}
};

bool PDDLParserAgent::parseConditions(ParserAgentDefinition agentDef, ConType conType)
{
	// Parse the pre/post conditions of the agent definition
	getNextToken();
	if (nextToke() == Input || nextToke() == Output || nextToke() == Parameters ||
		nextToke() == PreCondition || nextToke() == PostCondition || nextToke() == Effect)	// necessary? who knows?
	{
		cerr << "PDDLParerAgent - " << ConditionType::toStr(conType) << " parsing -- expression is empty -- returning";
		return false;
	}

	deque<int> condition;
	if (!parseExpression(condition, conType, agentDef))
		return false;
	switch (conType)
	{
	case ConType::PRECONDITION:
		agentDef.setPreconditions(condition);
		break;
		
	case ConType::POSTCONDITION:
		agentDef.setPostconditions(condition);
		break;

	default:
		cerr << "PDDLParserAgent - " << ConditionType::name << " parsing -- neither \"precondition\" or \"postcondition\"";
	}
	return true;
};

int PDDLParserAgent::parseAgent()
{
	// Parse the agent definition passed in constructor -- starts with "AgentName) (Domain"
	string agentName = getNextWord();
	if (agentName.empty())
	{
		cerr << "WARNING: PDDLParserAgent - agent parsing -- agent name is blank -- returning";
		return misformattedString;
	}

	int agentId = tokenize(agentName);
	if (isPredefined(agentId))
	{
		cerr << "WARNING: PDDLParserAgent - agent parsing -- agent name " << agentName << " is a reserved word -- returning";
		return misformattedString;
	}

	ParserAgentDefinition agentDef(agentName, agentId, tokenParser, domainParser);
	
	if (getNextToken() != CloseParen)
	{
		cerr << "WARNING: PDDLParserAgent - agent parsing -- missing close parenthesis -- returning";
		return misformattedString;
	}
	else if (getNextToken() == OpenParen)
	{
		if (getNextToken() != Domain)
		{
			cerr << "WARNING: PDDLParserAgent - agent parsing -- missing domain reserved word -- returning";
			return misformattedString;
		}
		if (!parseAgentDomains(agentDef))
			return misformattedString;

		getNextToken();
	}

	while (nextToke() != CloseParen)
	{
		switch (nextToke())
		{
		case Parameters:
			if (!parseIO(agentDef, ParType::PARAMETER))
				return misformattedString;
			break;
		case Input:
			if (!parseIO(agentDef, ParType::INPUT))
				return misformattedString;
			break;
		case Output:
			if (!parseIO(agentDef, ParType::OUTPUT))
				return misformattedString;
			break;
		case Effect:
			if (!parseEffects(agentDef))
				return misformattedString;
			break;
		case PreCondition:
			if (!parseConditions(agentDef, ConType::PRECONDITION))
				return misformattedString;
			break;
		case PostCondition:
			if (!parseConditions(agentDef, ConType::POSTCONDITION))
				return misformattedString;
			break;
		default:
			cerr << "WARNING: PDDLParserAgent - agent parsing -- unknown token " << nextToke();
			return misformattedString;
		}
	}

	agents.emplace(agentId, agentDef);
	return agentId;
};