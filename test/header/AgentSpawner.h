#ifndef AGENTSPAWNER_H
#define AGENTSPAWNER_H

#include <list>
#include <string>
#include <vector>

class SpawnRequest;

class AgentSpawner
{
private:
	const std::string exe_name = "cpp";
	const std::string cls_path_arg = "class";
	std::list<SpawnRequest> pendingSpawnRequests;
	std::string spawnerUuid = "";

public:
	void setSpawnerUuid(std::string uuid);
	void queueSpawnRequest(std::list<std::string> argList, std::list<std::string> envVars, const std::string logFile);
	void spawnAgent(std::list<std::string> argList, std::list<std::string> envVars, const std::string logFile);

private:
	std::list<std::string> generateCppArgList(std::vector<std::string> agentArgs);
	void spawnProcess(SpawnRequest request);
};

class SpawnRequest
{
private:
	std::list<std::string> argList;
	std::list<std::string> environmentVariables;	// C++ stores process as strings. A collection of processes can be stored in a string list
	std::string logFileBaseName;

public:
	SpawnRequest(std::list<std::string> args, std::list<std::string> vars, const std::string logFile)
	{
		argList = args;
		environmentVariables = vars;
		logFileBaseName = logFile;
	}

	std::list<std::string> getArgList() { return argList; };
	std::list<std::string> getEnvironmentVariables() { return environmentVariables; };
	std::string getLogFileBaseName() { return logFileBaseName; };
};

#endif