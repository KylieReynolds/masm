#include "../header/ZyreCommunicationLayer.h"

#include <poco/Exception.h>
#include <poco/Net/SocketAddress.h>
#include <poco/String.h>
using namespace std;

ZyreCommunicationLayer::ZyreCommunicationLayer(const string agentDef, const char* agentClass, ArgumentParser argParse)
{
	argumentParser = argParse;
	agentDefintion = agentDef;

	string pyrePort = argParse.getArgumentOrDefault("pyre-port", to_string(default_port));
	string pyreEvasive = argParse.getArgumentOrDefault("pyre-evasive-timeout", to_string(default_evasive_timeout));
	string pyreExpired = argParse.getArgumentOrDefault("pyre-expired-timeout", to_string(default_expired_timeout));
	string pyreInterface = argParse.getArgumentOrDefault("pyre-interface", default_network_interface);
	string pyreVerbose = argParse.getArgumentOrDefault("pyre-enable-verbose", default_verbose);
	const char* agentName = argParse.getArgumentOrDefault("agent-name", agentBase->default_name).c_str();

	Poco::Net::SocketAddress ipAddr;
	string addr = ipAddr.host().toString() + ":" + argParse.getArgumentOrDefault("webagent-port", "8054");	//8054 = webdebugserver default port

	zyre = zyre_new(agentName);
	zyre_set_header(zyre, node_name, agentName);
	zyre_set_header(zyre, description, agentDef.c_str());
	zyre_set_header(zyre, cls_name, agentClass);
	zyre_set_header(zyre, web_addr, addr.c_str());

	if(!pyreInterface.empty())
		zyre_set_interface(zyre, pyreInterface.c_str());
	if(pyreVerbose.compare("true") == 0)
		zyre_set_verbose(zyre);
	zyre_set_port(zyre, stoi(pyrePort));
	zyre_set_evasive_timeout(zyre, stoi(pyreEvasive));
	zyre_set_expired_timeout(zyre, stoi(pyreExpired));
	myIdentity = zyre_uuid(zyre);
};

void ZyreCommunicationLayer::notifyReceive(CommunicationMessageType commMessageType, string sender, string senderName, string group, string agentDef, char data[])
{
	if(commInputObserver == nullptr)
		return;

	switch(commMessageType)
	{
	case CommunicationMessageType::commEnter:
		commInputService.startFunc([&] { commInputObserver->processEnter(sender, senderName, agentDef); });
		break;
	case CommunicationMessageType::commExit:
		commInputService.startFunc([&] { commInputObserver->processExit(sender, senderName); });
		break;
	case CommunicationMessageType::commLeave:
		commInputService.startFunc([&] { commInputObserver->processLeave(sender, senderName, group); });
		break;
	case CommunicationMessageType::commShout:
		commInputService.startFunc([&] { commInputObserver->processShout(sender, senderName, group, data); });
		break;
	case CommunicationMessageType::commWhisper:
		commInputService.startFunc([&] { commInputObserver->processWhisper(sender, senderName, data); });
		break;
	default:
		throw invalid_argument("Invalid communication message type: " + commMessStr(commMessageType));
	}
};

void ZyreCommunicationLayer::notifyUuid(string uuid)
{
	if(commInputObserver == nullptr)
		return;
	commInputService.startFunc([&] { commInputObserver->processUuid(uuid); });
};

void ZyreCommunicationLayer::processSend(CommunicationMessageType commMessageType, string receiver, char data[])
{
	sendWorker->addData(commMessageType, receiver, data);
};

void ZyreCommunicationLayer::start()
{
	clog << "INFO: (" << agentName << ") Starting communications";
	int status = zyre_start(zyre);
	if(status != 0)
		throw invalid_argument("failed to start zyre node");

	notifyUuid(myIdentity);	// pass uuid to observer
	// start the tasks
	zyreOutgoingService.start(*sendWorker);
	zyreIncomingService.start(*receiveWorker);

	// stops all running threads and waits for their completion
	zyreOutgoingService.stopAll();
	zyreIncomingService.stopAll();
};

void ZyreCommunicationLayer::stop()
{
	// wait for tasks to finish
	zyreIncomingService.joinAll();

	try
	{
		commInputService.join(5000);
	}
	catch(Poco::TimeoutException ex)
	{
		cerr << ex.displayText();
	}

	zyre_stop(zyre);
	zyre_destroy(&zyre);
};

void SendWorker::addData(CommunicationMessageType commMessageType, std::string receiver, char data[])
{
	m.lock();
	myObject obj = {commMessageType, receiver, data};
	sendData.emplace(obj);
	vector<Poco::Thread>::iterator it;
	for(it = sendLock.begin(); it != sendLock.end(); it++)	// Wakeup all threads
	{
		Poco::Thread& thr = *it;
		thr.wakeUp();
	}
	m.unlock();
};

void SendWorker::stop()
{
	running = false;
	m.lock();
	vector<Poco::Thread>::iterator it;
	for(it = sendLock.begin(); it != sendLock.end(); it++)	// Wakeup all threads
	{
		Poco::Thread& thr = *it;
		thr.wakeUp();
	}
	m.unlock();
};

void SendWorker::run()
{
	running.exchange(true);
	CommunicationMessageType cCommMessageType;
	string rReceiver;
	char* dData;

	while(running)
	{
		waitForData();

		m.lock();
		if(sendData.empty())
			continue;
		myObject send = sendData.front();
		sendData.pop();
		cCommMessageType = send.commMessageType;
		rReceiver = send.receiver;
		dData = send.data;
		m.unlock();

		debugSendPrint(cCommMessageType, rReceiver);
		string group = dData;
		zmsg_t* msg = zmsg_new();
		switch(cCommMessageType)
		{
		case CommunicationMessageType::commJoin:
			zyre_join(zyre, group.c_str());
			break;
		case CommunicationMessageType::commLeave:
			zyre_leave(zyre, rReceiver.c_str());
			break;
		case CommunicationMessageType::commShout:
			zmsg_pushmem(msg, dData, sizeof(dData));
			zyre_shout(zyre, rReceiver.c_str(), &msg);
			break;
		case CommunicationMessageType::commWhisper:
			zmsg_pushmem(msg, dData, sizeof(dData));
			zyre_whisper(zyre, rReceiver.c_str(), &msg);
			break;
		default:
			cerr << "WARNING: Invalid communication message type";
		}
	}
};

void SendWorker::waitForData()
{
	bool isEmpty;

	m.lock();
	isEmpty = sendData.empty();
	m.unlock();

	if(isEmpty)
		if(m.tryLock())
			sendLock[0].yield();
};

void SendWorker::debugSendPrint(CommunicationMessageType commMessageType, string receiver)
{
	string myName = zcl->getAgentName();
	string theirName = zcl->getUuidMap().at(receiver);
	string message = "(" + myName + ") processSent " + commMessStr(commMessageType) + " ";
	if(theirName.empty())
		message += receiver;
	else
		message += theirName;
	clog << "FINE: " << message;
};

void ReceiveWorker::run()
{
	while(running)
	{
		zyre_event_t* zEvent = zyre_event_new(zyre);
		string command = zyre_event_type(zEvent);
		logAndUpdate(zEvent, command);
		string uuid = zyre_event_peer_uuid(zEvent);
		string name = zyre_event_peer_name(zEvent);
		string group = zyre_event_group(zEvent);
		char* data = zmsg_popstr(zyre_event_msg(zEvent));

		Poco::toLowerInPlace(command);
		if(command.compare("ENTER") == 0)
			zcl->notifyReceive(CommunicationMessageType::commEnter, uuid, name, group, zyre_event_header(zEvent, zcl->getDescription()), {});
		else if(command.compare("EXIT") == 0)
			zcl->notifyReceive(CommunicationMessageType::commExit, uuid, name, group, "", {});
		else if(command.compare("JOIN") == 0)
			clog << "INFO: Currently do not handle join responses...";
		else if(command.compare("LEAVE") == 0)
			zcl->notifyReceive(CommunicationMessageType::commLeave, uuid, name, group, "", {});
		else if(command.compare("SHOUT") == 0)
			zcl->notifyReceive(CommunicationMessageType::commShout, uuid, name, group, "", data);
		else if(command.compare("STOP") == 0)
			running = false;
		else if(command.compare("WHISPER") == 0)
			zcl->notifyReceive(CommunicationMessageType::commWhisper, uuid, name, group, "", data);
		else
			clog << "WARNING: unhandled command: " << command;
	}
};

void ReceiveWorker::logAndUpdate(zyre_event_t* zEvent, string command)
{
	string uuid = zyre_event_peer_uuid(zEvent);
	string name = zyre_event_peer_name(zEvent);
	string group = zyre_event_group(zEvent);
	
	if(zcl->getUuidMap().find(uuid) == zcl->getUuidMap().end())
		zcl->getUuidMap().emplace(uuid, name);
	
	if(!group.empty())
		clog << "INFO: (" << zcl->getAgentName() << ") (command: " << command << ") (peer: " << uuid << "/" << name << ") (group: " << group << ")";
	else
		clog << "INFO: (" << zcl->getAgentName() << ") (command: " << command << ") (peer: " << uuid << "/" << name << ")";
};