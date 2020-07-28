#ifndef MAS_H
#define MAS_H

#include "AgentFactory.h"
#include "AgentSpawner.h"
#include "ArgumentParser.h"
#include "HitmanAgent.h"
#include "SpyAgent.h"

class Mas
{
private:
	void parseAgentXml(std::string filename);
	std::map<std::string, std::string> loadProperties(std::string filename);
	void stopAgent(std::string uuid, std::string port);
	void startAgent(std::string agentname, std::map<std::string, std::string> environmenthash, std::list<std::string> args);
	std::list<std::string> listenAgents(std::string port);
	std::vector<std::string> convertParameterHash(std::map<std::string, std::string> params);
	std::list<std::string> convertParameterList(std::map<std::string, std::string> params);
};

#endif