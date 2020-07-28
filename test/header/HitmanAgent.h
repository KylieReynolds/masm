#ifndef HITMANAGENT_H
#define HITMANAGENT_H

#include "AgentBase.h"
#include "ArgumentParser.h"
#include <typeinfo>

class HitmanAgent : public AgentBase
{
private:
	HitmanAgent* hAgent = new HitmanAgent();	// needed so that default_agent_name will return HitmanAgent class name
	ArgumentParser hitmanArgParse;

public:
	const std::string h_default_agent_name = typeid(&hAgent).name();
	const std::string h_agent_definition = "(define (agent HitmanAgent) (:domain Vector) :precondtition (false) :postcondition (false))";

	HitmanAgent() {};	// Default constructor
	HitmanAgent(ArgumentParser argumentParser);
	explicit HitmanAgent(const AgentBase& ab);	// converting constructor

	void sendStopRequest(std::string uuid);
};

#endif 