#include "../header/AgentBase.h"

using namespace std;

AgentBase::AgentBase(ArgumentParser argumentParser)
{
	agentName = argumentParser.getArgumentOrDefault("agent-name", default_name);
	argParser = argumentParser;
	processTaskUuid(argParser.getArgumentOrDefault(task_uuid_arg, ""));
	if(argParser.hasArgument(spawn_uuid_arg))
		spawnAgentResponseFuture = spawnPool;
	spawnPool->cancel();
};
AgentBase::AgentBase(const AgentBase& ab)
{
	agentName = ab.agentName;
	argParser = ab.argParser;
	processTaskUuid(argParser.getArgumentOrDefault("task-uuid", ""));
	if(argParser.hasArgument("spawn-uuid"))
		spawnAgentResponseFuture = spawnPool;
	spawnPool->cancel();
};
AgentBase& AgentBase::operator=(const AgentBase& ab)
{
	agentName = ab.agentName;
	argParser = ab.argParser;
	processTaskUuid(argParser.getArgumentOrDefault("task-uuid", ""));
	if(argParser.hasArgument("spawn-uuid"))
		spawnAgentResponseFuture = spawnPool;
	spawnPool->cancel();
	return* this;
};

void AgentBase::notifySend(NodeMessageType nodeMessageType, string receiver, WrapperMessage wrapper)
{
	if(nodeOutputObserver == nullptr)
		return;
	nodeOutputObserver->processSend(nodeMessageType, receiver, wrapper);
};

void AgentBase::processStop(std::string sender, std::string senderName, WrapperMessage wrapper)
{
	if(!wrapper.has_stoprequest())
		return;
	clog << "INFO: (" << agentName << ") Received stop request from " << senderName;
	if(confirmStopRequest())
		stop();
};

void AgentBase::processUuid(const std::string uuid)
{
	myIdentity = uuid;
	agentSpawner.setSpawnerUuid(uuid);
	clog << "UUID: " << myIdentity;
};

const void AgentBase::start()
{
	if(nodeOutputObserver != nullptr)
		nodeOutputObserver->start();
	startAgent();
};

const void AgentBase::stop()
{
	if(spawnAgentResponseFuture != nullptr)
		spawnAgentResponseFuture->cancel();
	stopAgent();
	if(nodeOutputObserver != nullptr)
		nodeOutputObserver->stop();
};

AgentSpawner AgentBase::getAgentSpawner()
{
	if(!getIdentity().empty())
		agentSpawner.setSpawnerUuid(getIdentity());
	return agentSpawner;
};

void AgentBase::sendPublication(ExpressionList expressionList, WrapperMessage wrapper)
{
	if(nodeOutputObserver == nullptr)
		return;
	string group = nodeOutputObserver->getPublicationGroup(expressionList);
	if(!group.empty())
		nodeOutputObserver->processSend(NodeMessageType::nodeShout, group, wrapper);
};

void AgentBase::sendMessage(std::string peer, WrapperMessage wrapper)
{
	if(nodeOutputObserver == nullptr)
		return;
	nodeOutputObserver->processSend(NodeMessageType::nodeWhisper, peer, wrapper);
};

void SpawnAgentResponseRunnable::run()
{
	if(spawnerUuid.empty())
		return;
	try
	{
		waitForUuid();
		sendSpawnRequest();
	}
	catch(exception ex)
	{
		cerr << "WARNING: Interrupted warning for UUID" << ex.what();
	}
};

void SpawnAgentResponseRunnable::sendSpawnRequest()
{
	SpawnAgentResponse spawnMsg;
	spawnMsg.set_agent_definition(agentDef.getDefinition());
	spawnMsg.set_uuid(myIdentity);

	WrapperMessage wrapper;
	wrapper.set_allocated_spawnagentresponse(&spawnMsg);

	clog << "INFO: (" << agentName << ") Sending spawn agent response to " << spawnerUuid;
	clog << "INFO: (" << agentName << ") Spawn agent response message: " << wrapper.DebugString();
	sendMessage(spawnerUuid, wrapper);
};