#ifndef ZYRECOMMUNICATIONLAYER_H
#define ZYRECOMMUNICATIONLAYER_H

#include "AgentBase.h"
#include "ArgumentParser.h"
#include "Communication.h"
#include <atomic>
#include <queue>

#include <poco/Mutex.h>
#include <poco/Runnable.h>
#include <poco/Thread.h>
#include <poco/ThreadPool.h>
#include <zeromq/czmq/zmsg.h>
#include <zeromq/zyre/zyre.h>

class SendWorker;
class ReceiveWorker;

class ZyreCommunicationLayer : public CommunicationInputObservable, public CommunicationOutputObserver
{
private:
	// Defined constants
	const int default_port = 5670;
	const int default_evasive_timeout = 9999999;
	const int default_expired_timeout = 9999999;
	const std::string default_network_interface = "";
	const std::string default_verbose = "false";
	const char* description = "description";
	const char* node_name = "node_name";
	const char* web_addr = "web_addr";
	const char* cls_name = "agent_class_name";

	Poco::Thread commInputService;
	Poco::ThreadPool zyreIncomingService;
	Poco::ThreadPool zyreOutgoingService;

	zyre_t* zyre;
	AgentBase* agentBase;
	ArgumentParser argumentParser;
	CommunicationInputObserver* commInputObserver;
	SendWorker* sendWorker;
	ReceiveWorker* receiveWorker;

	std::string agentDefintion;
	std::string agentName;
	std::string myIdentity;
	std::unordered_map<std::string, std::string> uuidNameMap;

public:
	ZyreCommunicationLayer(const std::string agentDef, const char* agentClass, ArgumentParser argParse);

	// CommunicationInputObservable methods
	void setCommunicationInputObserver(CommunicationInputObserver& commInObs) override { *commInputObserver = commInObs; };
	void notifyReceive(CommunicationMessageType commMessageType, std::string sender, std::string senderName, std::string group, std::string agentDef, char data[]) override;
	void notifyUuid(std::string uuid) override;

	// CommunicationOutputObserver methods
	void processSend(CommunicationMessageType commMessageType, std::string receiver, char data[]) override;
	void start() override;
	void stop() override;

	const char* getDescription() { return description; };
	std::string getAgentName() { return agentName; };
	std::unordered_map<std::string, std::string> getUuidMap() { return uuidNameMap; };
};

// In lieu of the Java's object class
struct myObject
{
	CommunicationMessageType commMessageType;
	std::string receiver;
	char* data;
};

class SendWorker : public Poco::Runnable
{
private:
	Poco::Mutex m;
	std::atomic_bool running = false;
	std::queue<myObject> sendData;
	std::vector<Poco::Thread> sendLock;
	zyre_t* zyre;
	ZyreCommunicationLayer* zcl;

public:
	void addData(CommunicationMessageType commMessageType, std::string receiver, char data[]);
	void stop();
	void run() override;

private:
	void waitForData();
	void debugSendPrint(CommunicationMessageType commMessageType, std::string receiver);
};

class ReceiveWorker : public Poco::Runnable
{
private:
	std::atomic_bool running = true;
	zyre_t* zyre;
	ZyreCommunicationLayer* zcl;

public:
	void run() override;
	void logAndUpdate(zyre_event_t* zEvent, std::string command);
};

#endif