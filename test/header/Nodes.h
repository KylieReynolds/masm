/**********
 * Combines the files:
 *		NodeInputObservable.java
 *		NodeInputObserver.java
 *		NodeMessageType.java
 *		NodeOuputObservable.java
 *		NodeOutputObserver.java
 **********/

#ifndef NODES_H
#define NODES_H

#include "ExpressionList.h"

#include <protos/Communication.pb.h>
using namespace mas_middleware::protos;

enum class NodeMessageType
{
    nodeLeave,
    nodeShout,
    nodeWhisper
};
std::string nodeMessageString(NodeMessageType nmt)
{
    switch(nmt)
    {
    case NodeMessageType::nodeLeave:
        return "LEAVE";
    case NodeMessageType::nodeShout:
        return "SHOUT";
    case NodeMessageType::nodeWhisper:
        return "WHISPER";
    default:
        return "";
    }
};

/**********
 * Interfaces are made by all methods being in the format:
 * virtual void function() = 0
 **********/

class NodeInputObserver
{
public:
    virtual void processData(std::string sender, std::string senderName, std::string groupName, WrapperMessage wrapper) = 0;
    virtual void processPeerArrived(std::string peer, std::string peerName) = 0;
    virtual void processPeerDeparted(std::string peer, std::string peerName) = 0;
    virtual void processStop(std::string sender, std::string senderName, WrapperMessage wrapper) = 0;
    virtual void processUuid(std::string uuid) = 0;
    virtual void processTaskUuid(std::string uuid) = 0;
    virtual bool confirmPartial(ExpressionList partialConditions) = 0;
};

class NodeInputObservable
{
public:
    virtual void setNodeInputObserver(NodeInputObserver& observer) = 0;
    virtual void notifyPeerArrived(std::string peer, std::string peerName) = 0;
    virtual void notifyPeerDeparted(std::string peer, std::string peerName) = 0;
    virtual void notifyReceive(NodeMessageType messageType, std::string sender, std::string senderName, std::string groupName, WrapperMessage wrapper) = 0;
    virtual void notifyStop(std::string sender, std::string senderName, WrapperMessage wrapper) = 0;
};

class NodeOutputObserver
{
public:
    virtual void processSend(NodeMessageType messageType, std::string receiver, WrapperMessage wrapper) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual std::string getPublicationGroup(ExpressionList expressionList) = 0;
};

class NodeOutputObservable
{
public:
    virtual void setNodeOutputObserver(NodeOutputObserver& observer) = 0;
    virtual void notifySend(NodeMessageType messageType, std::string receiver, WrapperMessage wrapper) = 0;
};

#endif
