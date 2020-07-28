#ifndef AGENTFACTORY_H
#define AGENTFACTORY_H

#include "AgentBase.h"
#include "ArgumentParser.h"
#include "NodeCommunicationLayer.h"
#include "ZyreCommunicationLayer.h"
#include <typeinfo>

class AgentFactory
{
public:
	AgentFactory() {};
	static AgentBase getAgent(std::string agentClassName, ArgumentParser argumentParser);
	static AgentBase getAgent(AgentBase agentClass, ArgumentParser argumentParser);

private:
	static std::string getFactoryConstructor(AgentBase agentClass);
};

#endif