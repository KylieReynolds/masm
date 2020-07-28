#ifndef AGENTBASE_H
#define AGENTBASE_H

#include "AgentSpawner.h"
#include "ArgumentParser.h"
#include "Nodes.h"
#include "PDDLParser.h"
#include <poco/Util/TimerTask.h>
#include <poco/Runnable.h>
#include <protos/Communication.pb.h>

using namespace mas_middleware::protos;
using pocoTimer = Poco::Util::TimerTask;

class HitmanAgent;

class AgentBase : public NodeInputObserver, public NodeOutputObservable
{
protected:
    std::atomic_int jobId;
    std::atomic_int sequence;
    std::string myIdentity;   // AtomicReference<String> in java
    std::string agentName;
    std::string taskUuid;

    PDDLParser parser;
    ParserAgentDefinition agentDef;
    ArgumentParser argParser;
    NodeOutputObserver* nodeOutputObserver;

private:
    AgentSpawner agentSpawner;
    pocoTimer* spawnPool;
    pocoTimer* spawnAgentResponseFuture;
    pocoTimer* uuidCoundDownLatch;

public:
    const std::string default_name = "default";
    const std::string spawn_uuid_arg = "spawn-uuid";
    const std::string task_uuid_arg = "task-uuid";
    AgentBase() {}; // Default constructor
    AgentBase(ArgumentParser argumentParser);
    AgentBase(const AgentBase& ab); // Copy constructor
    AgentBase& operator=(const AgentBase& ab);  // Assignment operator

    // NodeOutputObservable methods
    void setNodeOutputObserver(NodeOutputObserver& nodeOutObs) override { *nodeOutputObserver = nodeOutObs; };
    void notifySend(NodeMessageType nodeMessageType, std::string receiver, WrapperMessage wrapper) override;

    // NodeInputObserver methods - Empty because they do not need to do things (idk why these are here)
    void processData(std::string sender, std::string senderName, std::string groupName, WrapperMessage wrapper) override {};
    void processPeerArrived(std::string peer, std::string peerName) override {};
    void processPeerDeparted(std::string peer, std::string peerName) override {};
    void processStop(std::string sender, std::string senderName, WrapperMessage wrapper) override;
    void processUuid(std::string uuid) override;
    void processTaskUuid(const std::string tuuid) override { taskUuid = tuuid; };
    bool confirmPartial(ExpressionList partialConditions) override { return true; };

    // Class methods
    void setPddlParser(PDDLParser pddlParser) { parser = pddlParser; };
    void setAgentDefinition(ParserAgentDefinition agentDefinition) { agentDef = agentDefinition; };
    std::string getIdentity() { return myIdentity; };
    std::string getAgentName() { return agentName; };
    std::string getTaskUuid() { return taskUuid; };
    ArgumentParser getArgumentParser() { return argParser; };
    void leaveGroup(std::string peer) { notifySend(NodeMessageType::nodeLeave, peer, {}); };
    const void start();
    const void stop();
    void waitForUuid() { uuidCoundDownLatch->release(); };

protected:
    AgentSpawner getAgentSpawner();
    bool confirmStopRequest() { return true; };
    const int getNextSequence() { return sequence.operator++(); };
    std::string getNextJobId() { return myIdentity + std::to_string(jobId.operator++()); };
    void sendPublication(ExpressionList expressionList, WrapperMessage wrapper);
    void sendPublication(WrapperMessage wrapper) { sendPublication({}, wrapper); };
    void sendMessage(std::string peer, WrapperMessage wrapper);
    void startAgent() {};   // Empty method
    void stopAgent() {};   // Empty method
};

class SpawnAgentResponseRunnable : public Poco::Runnable, AgentBase
{
private:
    std::string spawnerUuid;

public:
    SpawnAgentResponseRunnable(std::string spawnUuid) { spawnUuid = spawnerUuid; };
    void run();
    void sendSpawnRequest();
};

#endif 