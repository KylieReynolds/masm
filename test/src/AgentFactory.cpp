#include "../header/AgentFactory.h"
#include "AgentUtil.cpp"
#include <exception>
#include <poco/String.h>
using namespace std;

AgentBase AgentFactory::getAgent(string agentClassName, ArgumentParser argumentParser)
{
	/*********
	 *	Java's class.forName and class.isAssignableFrom methods are used in the original Java code:
	 *	forName returns a class object and isAssignableFrom determines if two class objects are the same.
	 *	There are no equivalent methods in C++ so the workaround is:
	 *		- forName(agentClassName) is represented the string agentClassName
	 *		- check if the specified class is agentbase by checking if the agentClassName (ignoring case
	 *			by converting to all lowercase) is agentbase
	 *********/
	AgentBase clazz = AgentBase(argumentParser);
	if(Poco::toLower(agentClassName) == "agentbase")
		return getAgent(clazz, argumentParser);
	else
		return {};	// return null
};

AgentBase AgentFactory::getAgent(AgentBase agentClass, ArgumentParser argumentParser)
{
	//Since C++ does not have a Class class, Java's Class<? extends AgentBase> is represented by using an AgentBase constructor
	const string agentClassName = typeid(agentClass).name();
	clog << "INFO: Attempting to create new agent for type (" << agentClassName << ")";

	// provide default agent-name if not provided
	if(!argumentParser.hasArgument("agent-name"))
		argumentParser.addArgument("--agent-name", agentClassName);

	// get agent name and definition
	const string agentName = argumentParser.getArgument("agent-name");
	string agentDefinitionString;
	AgentUtil agentUtil;
	if(argumentParser.hasArgument("agent-definition-file"))
		agentDefinitionString = agentUtil.readDefinitionFile(argumentParser.getArgument("agent-definition-file"));
	else if(argumentParser.hasArgument("agent-definition-string"))
		agentDefinitionString = argumentParser.getArgument("agent-definition-string");
	else
		throw invalid_argument("agent definition not provided");

	// get domain definition(s)
	if(!argumentParser.hasArgument("domain-definition-file"))
		throw invalid_argument("domain definition(s) not provided");
	list<string> domainFileNames = argumentParser.getArgumentList("domain-definition-file");

	// create parser, parse agent and get the ID, get the agent definition, and create vector of parsed domains 
	PDDLParser parser;
	list<int> agentIds = parser.parseString(agentDefinitionString);
	ParserAgentDefinition agentDef = parser.getParserAgent().getAgent(agentIds.front());
	vector<list<int>> parseDomains;
	for(string domainFile : domainFileNames)
		parseDomains.emplace_back(parser.parseString(agentUtil.readDefinitionFile(domainFile)));

	// create objects
	AgentBase* agent = new AgentBase();
	string constructor = getFactoryConstructor(agentClass);
	if(!constructor.empty())
		*agent = AgentBase(argumentParser);
	else
		cerr << "SEVERE: agent " << agentClassName << " does not have a factory constructor";

	if(agent == nullptr)
		return {};	// return null

	// create layers after the agent in case creation of agent fails
	NodeCommunicationLayer* nodeLayer = new NodeCommunicationLayer(argumentParser);
	ZyreCommunicationLayer* commLayer = new ZyreCommunicationLayer(agentDef.getDefinition(), agentName.c_str(), argumentParser);

	// setup communication between ZyreCommunicationLayer and NodeCommunicationLayer
	commLayer->setCommunicationInputObserver(*nodeLayer);
	nodeLayer->setCommunicationOutputObserver(*commLayer);

	// setup communication between NodeCommunicationLayer and AgentBase
	agent->setNodeOutputObserver(*nodeLayer);
	nodeLayer->setNodeInputObserver(*agent);

	// set agent definition, parser, and task uuid... setAgentDefinition MUST be before setPDDLParser
	agent->setAgentDefinition(agentDef);
	nodeLayer->setAgentDefinition(agentDef);
	agent->setPddlParser(parser);
	nodeLayer->setPddlParser(parser);
	nodeLayer->processUuid(agent->getTaskUuid());

	return *agent;
};

string AgentFactory::getFactoryConstructor(AgentBase agentClass)
{
	/*********
	 *	C++ has no equivalent for Java's Constructor class. Java getConstructor method returns a
	 *	constructor object that can be turned into a string that includes the constructor with parameters.
	 *	Rather than creating an object, this information is put directly into a string here.
	 *********/
	ArgumentParser argumentParser;
	string constructor = typeid(agentClass).name();
	constructor += '(' + typeid(argumentParser).name() + ')';
	return constructor;
};