#ifndef NODECOMMUNICAITONLAYER_H
#define NODECOMMUNICATIONLAYER_H

#include "AgentBase.h"
#include "ArgumentParser.h"
#include "Communication.h"
#include "Nodes.h"
#include "PDDLParser.h"
#include "Tokens.h"

#include <atomic>
#include <chrono>
#include <poco/Util/Timer.h>
#include <poco/Util/TimerTask.h>
#include <protos/Communication.pb.h>
#include <thread/concurrent_unordered_map.h>

namespace gp = google::protobuf;
using pocoTimer = Poco::Util::TimerTask;

class Subscription
{
public:
	std::string publisher;
	std::string group;
	long creationTime;
	std::atomic_int messageCount;
	Subscription(std::string publish, std::string grp, long crTime);
	Subscription(const Subscription& sub);	//Copy constructor
};

class Publication
{
public:
	std::set<std::string> subscribers;
	std::string group;
	ExpressionList conditions;
	long creationTime;
	std::atomic_int messageCount;
	Publication(std::string grp, ExpressionList conds, long crTime);
	Publication(const Publication& pub);	// Copy constructor
	Publication& operator=(const Publication& pub)
	{
		group = pub.group;
		conditions = pub.conditions;
		creationTime = pub.creationTime;
		return *this;
	};	// Assignment operator
};

class NodeCommunicationLayer : public CommunicationInputObserver, public CommunicationOutputObservable, public NodeInputObservable, public NodeOutputObserver, public Tokens
{
protected:
	CommunicationOutputObserver* commOutputObserver;
	NodeInputObserver* nodeInputObserver;
	PDDLParser parser;
	ParserAgentDefinition agentDefinition;

	std::string identity = "";	// null string
	std::string taskuuid = "";	// null string
	std::atomic_int sequence;

	tbb::concurrent_unordered_map<std::string, Subscription> subscriptions;
	tbb::concurrent_unordered_map<std::string, Publication> publications;
	tbb::concurrent_unordered_map<std::string, pocoTimer::Ptr> scheduledTransDataTasks;
	pocoTimer::Ptr execService;

private:
	std::string agentName;
	bool join;
	AgentBase ab;

public:
	NodeCommunicationLayer(ArgumentParser argumentParser);
	void setPddlParser(PDDLParser pddlParser) { parser = pddlParser; };
	void setAgentDefinition(ParserAgentDefinition agentDef) { agentDefinition = agentDef; };

	// CommunicationOutputObservable methods
	void setCommunicationOutputObserver(CommunicationOutputObserver& commOutObs) override;
	void notifySend(CommunicationMessageType commMessageType, std::string receiver, char data[]) override;

	// CommunicationInputObserver methods
	void processEnter(std::string sender, std::string senderName, std::string agentDef) override;
	void processExit(std::string sender, std::string senderName) override;
	void processLeave(std::string sender, std::string senderName, std::string groupName) override;
	void processShout(std::string sender, std::string senderName, std::string groupName, char data[]) override;
	void processWhisper(std::string sender, std::string senderName, char data[]) override;
	void processUuid(std::string uuid) override;

	// NodeInputObservable methods
	void setNodeInputObserver(NodeInputObserver& nodeInObs) override;
	void notifyPeerArrived(std::string peer, std::string peerName) override;
	void notifyPeerDeparted(std::string peer, std::string peerName) override;
	void notifyReceive(NodeMessageType nodeMessageType, std::string sender, std::string senderName, std::string groupName, WrapperMessage wrapper) override;
	void notifyStop(std::string sender, std::string senderName, WrapperMessage wrapper) override;

	// NodeOutputObserver methods
	std::string getPublicationGroup(ExpressionList expressionList) override;
	void processSend(NodeMessageType messageType, std::string receiver, WrapperMessage wrapper) override;
	void start() override;
	void stop() override;

private:
	bool isConsistentWithPDDL(gp::Map<std::string, gp::Value> metadata);

protected:
	void processCheckCompatibility(std::string sender, std::string senderName, std::string agentDefString);
	void processData(NodeMessageType nodeMessageType, std::string sender, std::string senderName, std::string groupName, char data[]);
	void requestSubscription(std::string peerUuid);
	void processConnectionRequest(std::string sender, WrapperMessage wrapper);
	void processConnectionResponse(WrapperMessage wrapper);
	void setupTransactionRateResponse(std::string sender, WrapperMessage wrapper);
	void sendTransactionSummary(std::string recipient);
	AgentCompatibility checkGoodSubscriber(std::string peerUuid, std::string peerAgentDef);
	void addSubscriber(std::string peerUuid, std::vector<ExpressionList> conditionList);
	Publication getExpressionListEqual(ExpressionList expressionList);
	void addSubscription(std::string group, std::string peerUuid);
	bool confirmPartial(ExpressionList expressionList);
	void processTaskUuid(std::string uuid);
};

#endif
