#ifndef SPYAGENT_H
#define SPYAGENT_H

#include "AgentBase.h"
#include "AgentFactory.h"
#include "ArgumentParser.h"
#include <typeinfo>
#include <thread/concurrent_unordered_map.h>

class SpyAgent : public AgentBase
{
private:
	SpyAgent* sAgent = new SpyAgent(); // needed so that default_agent_name will return SpyAgent class name
	ArgumentParser spyArgParser;

	tbb::concurrent_unordered_map<std::string, std::string> peers;

public:
	const std::string s_default_agent_name = typeid(&sAgent).name();
	const std::string s_agent_definition = "(define (agent SpyAgent) (:domain Vector) :precondition (true) :postcondition (ture))";

	SpyAgent() {};	// Default constructor
	SpyAgent(ArgumentParser argumentParser);
	explicit SpyAgent(const AgentBase& ab);	// converting constructor

	void processPeerArrived(std::string peer, std::string peerName) override;
	void processPeerDeparted(std::string peer, std::string peerName) override;
	int doQuery();
};

#endif