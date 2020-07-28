#include "../header/AgentSpawner.h"
#include "../header/AgentBase.h"

#include <iostream>
#include <fstream>
#include <poco/String.h>

using namespace std;

void AgentSpawner::setSpawnerUuid(string uuid)
{
	spawnerUuid = uuid;
	if(!spawnerUuid.empty())
		for(SpawnRequest request : pendingSpawnRequests)
			spawnProcess(request);
};

void AgentSpawner::queueSpawnRequest(list<string> argList, list<string> envVars, const string logFile)
{
	SpawnRequest request = SpawnRequest(argList, envVars, logFile);
	if(spawnerUuid.empty())
		pendingSpawnRequests.emplace_back(request);
	else
		spawnAgent(argList, envVars, logFile);
};

void AgentSpawner::spawnAgent(list<string> argList, list<string> envVars, const string logFile)
{
	vector<string> argVec;
	// Remove "spawn-uuid" from argList if it exists
	list<string>::iterator it;
	for(it = argList.begin(); it != argList.end(); it++)
	{
		AgentBase ab;
		string tmp = *it;
		argVec.push_back(tmp);	// convert argList to vector to utilize generateCppArgList
		if(tmp.compare(ab.spawn_uuid_arg) == 0)
			argList.erase(it);
	}

	string tmp2 = Poco::toLower(argList.front());
	if(tmp2.compare(exe_name) == 0)
		argList = generateCppArgList(argVec);
	
	SpawnRequest request = SpawnRequest(argList, envVars, logFile);
	spawnProcess(request);
};

list<string> AgentSpawner::generateCppArgList(vector<string> agentArgs)
{
	list<string> args;
	args.push_back(exe_name);

	int position = 0;
	vector<string>::iterator it;
	for(it = agentArgs.begin(); it != agentArgs.end(); it++)
	{
		string tmp = *it;
		if(tmp.compare("--" + cls_path_arg) == 0)
		{
			if(position == 0)
				break;
			args.push_back("-" + cls_path_arg);
			args.push_back(agentArgs.at(position++));
			agentArgs.erase(it);
		}
		position++;
	}
	args.push_back(agentArgs.at(1));

	// Remove duplicates
	sort(agentArgs.begin(), agentArgs.end());
	auto x = unique(agentArgs.begin(), agentArgs.end());
	agentArgs.erase(x, agentArgs.end());

	vector<string>::iterator agentIt;
	for(agentIt = agentArgs.begin(); agentIt != agentArgs.end(); agentIt++)
		args.push_back(*agentIt);

	return args;
};

void AgentSpawner::spawnProcess(SpawnRequest request)
{
	list<string> argList = request.getArgList();
	if(!spawnerUuid.empty())
	{
		AgentBase ab;
		argList.push_back("--" + ab.spawn_uuid_arg);
		argList.push_back(spawnerUuid);
	}

	list<string> processList = argList;
	if(!request.getEnvironmentVariables().empty())
		for(string entry : request.getEnvironmentVariables())
			processList.push_back(getenv(entry.c_str()));

	string logName;
	if(spawnerUuid.empty())
		logName = request.getLogFileBaseName() + ".log";
	else
		logName = spawnerUuid + "-" + request.getLogFileBaseName() + ".log";
	ofstream logFile(logName);
	for(string a : processList)
	{
		logFile << a;
		system(a.c_str());	// equivalent to Java's processbuilder start()
	}
};