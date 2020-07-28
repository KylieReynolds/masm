/**********
 * Combines the files:
 *		CommunicationInputObservable.java
 *		CommunicationInputObserver.java
 *		CommunicationMessageType.java
 *		CommunicationOuputObservable.java
 *		CommunicationOutputObserver.java
 **********/

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <string>

enum class CommunicationMessageType
{
	commEnter,
	commExit,
	commJoin,
	commLeave,
	commShout,
	commStop,
	commWhisper
};
std::string commMessStr(CommunicationMessageType cmt)
{
	switch(cmt)
	{
	case CommunicationMessageType::commEnter: return "ENTER";
	case CommunicationMessageType::commExit: return "EXIT";
	case CommunicationMessageType::commJoin: return "JOIN";
	case CommunicationMessageType::commLeave: return "LEAVE";
	case CommunicationMessageType::commShout: return "SHOUT";
	case CommunicationMessageType::commStop: return "STOP";
	case CommunicationMessageType::commWhisper: return "WHISPER";
	default: return "";
	}
}

/**********
 * Interfaces are made by all methods being in the format:
 * virtual void function() = 0
 **********/

class CommunicationInputObserver
{
public:
	virtual void processEnter(std::string sender, std::string senderName, std::string agentDefinition) = 0;
	virtual void processExit(std::string sender, std::string senderName) = 0;
	virtual void processLeave(std::string sender, std::string senderName, std::string groupName) = 0;
	virtual void processShout(std::string sender, std::string senderName, std::string groupName, char data[]) = 0;
	virtual void processWhisper(std::string sender, std::string senderName, char data[]) = 0;
	virtual void processUuid(std::string uuid) = 0;
};

class CommunicationInputObservable
{
public:
	virtual void setCommunicationInputObserver(CommunicationInputObserver& commInObserver) = 0;
	virtual void notifyReceive(CommunicationMessageType commMessageType, std::string sender, std::string senderName,
		std::string groupName, std::string agentDefinition, char data[]) = 0;
	virtual void notifyUuid(std::string uuid) = 0;
};

class CommunicationOutputObserver
{
public:
	virtual void processSend(CommunicationMessageType commMessageType, std::string receiver, char data[]) = 0;
	virtual void start() = 0;
	virtual void stop() = 0;
};

class CommunicationOutputObservable
{
public:
	virtual void setCommunicationOutputObserver(CommunicationOutputObserver& commOutObserver) = 0;
	virtual void notifySend(CommunicationMessageType commMessageType, std::string receiver, char data[]) = 0;
};

#endif