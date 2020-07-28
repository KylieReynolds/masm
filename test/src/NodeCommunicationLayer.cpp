#include "../header/NodeCommunicationLayer.h"
#include <poco/String.h>

using namespace std;

Subscription::Subscription(string publish, string grp, long crTime)
{
	publisher = publish;
	group = grp;
	creationTime = crTime;
};
Subscription::Subscription(const Subscription& sub)
{
	publisher = sub.publisher;
	group = sub.group;
	creationTime = sub.creationTime;
};

Publication::Publication(string grp, ExpressionList conds, long crTime)
{
	group = grp;
	conditions = conds;
	creationTime = crTime;
};
Publication::Publication(const Publication& pub)
{
	group = pub.group;
	conditions = pub.conditions;
	creationTime = pub.creationTime;
};

NodeCommunicationLayer::NodeCommunicationLayer(ArgumentParser argumentParser)
{
	agentName = argumentParser.getArgumentOrDefault("agent-name", ab.default_name);
	string s = argumentParser.getArgumentOrDefault("join", "true");
	if(Poco::toLower(s).compare("true") == 0)
		join = true;
	else
		join = false;
};

void NodeCommunicationLayer::setCommunicationOutputObserver(CommunicationOutputObserver& commOutObs)
{
	*commOutputObserver = commOutObs;
};

void NodeCommunicationLayer::notifySend(CommunicationMessageType commMessageType, string receiver, char data[])
{
	if(commOutputObserver == nullptr)
		return;
	commOutputObserver->processSend(commMessageType, receiver, data);
};

void NodeCommunicationLayer::processEnter(string sender, string senderName, string agentDef)
{
	clog << "FINE: (" << agentName << ") processEnter for " << senderName;
	processCheckCompatibility(sender, senderName, agentDef);
	notifyPeerArrived(sender, senderName);
};

void NodeCommunicationLayer::processExit(string sender, string senderName)
{
	clog << "FINE: (" << agentName << ") processExit for " << senderName;
	if(!scheduledTransDataTasks.at(sender)->isCancelled())
		scheduledTransDataTasks.at(sender)->cancel();	
	notifyPeerDeparted(sender, senderName);
};

void NodeCommunicationLayer::processLeave(string sender, string senderName, string groupName)
{
	clog << "FINE: (" << agentName << ") processLeave";
	if(publications.count(groupName) != 0)
	{
		Publication publication = publications.at(groupName);
		if(publication.subscribers.count(sender) != 0)
		{
			publication.subscribers.erase(sender);
			clog << "INFO: (" << agentName << ") removing " << senderName << " from group " << groupName;
		}
	}
};

void NodeCommunicationLayer::processShout(string sender, string senderName, string groupName, char data[])
{
	Subscription s = subscriptions.at(groupName);
	s.messageCount.operator++();
	processData(NodeMessageType::nodeShout, sender, senderName, groupName, data);
};

void NodeCommunicationLayer::processWhisper(string sender, string senderName, char data[])
{
	processData(NodeMessageType::nodeWhisper, sender, senderName, "", data);
};

void NodeCommunicationLayer::processUuid(string uuid)
{
	clog << "INFO: UUID: " << uuid;
	identity = uuid;
	if(agentName.compare(ab.default_name) == 0)
		agentName = uuid;

	if(nodeInputObserver != nullptr)
		nodeInputObserver->processUuid(identity);
};

void NodeCommunicationLayer::setNodeInputObserver(NodeInputObserver& nodeInObs)
{
	*nodeInputObserver = nodeInObs;
};

void NodeCommunicationLayer::notifyPeerArrived(string peer, string peerName)
{
	if(nodeInputObserver == nullptr)
		return;
	nodeInputObserver->processPeerArrived(peer, peerName);
};

void NodeCommunicationLayer::notifyPeerDeparted(string peer, string peerName)
{
	if(nodeInputObserver == nullptr)
		return;
	nodeInputObserver->processPeerDeparted(peer, peerName);
};

void NodeCommunicationLayer::notifyReceive(NodeMessageType nodeMessageType, string sender, string senderName, string groupName, WrapperMessage wrapper)
{
	if(nodeInputObserver == nullptr)
		return;
	nodeInputObserver->processData(sender, senderName, groupName, wrapper);
};

void NodeCommunicationLayer::notifyStop(string sender, string senderName, WrapperMessage wrapper)
{
	if(nodeInputObserver == nullptr)
		return;
	nodeInputObserver->processStop(sender, senderName, wrapper);
};

string NodeCommunicationLayer::getPublicationGroup(ExpressionList expressionList)
{
	if(!expressionList.empty())
	{
		tbb::concurrent_unordered_map<string, Publication>::iterator itPub;
		for(itPub = publications.begin(); itPub != publications.end(); itPub++)
		{
			Publication pub = itPub->second;
			if(pub.conditions.containsExpressionList(expressionList))
				return pub.group;
		}
	}
	else if(expressionList.empty() && publications.size() == 1) // If exactly one group and blank expressionList, return group
	{
		tbb::concurrent_unordered_map<string, Publication>::iterator itPub;
		for(itPub = publications.begin(); itPub != publications.end(); itPub++)
			return itPub->second.group;
	}
	return "";	// return null
};

void NodeCommunicationLayer::processSend(NodeMessageType nodeMessageType, string receiver, WrapperMessage wrapper)
{
	if(commOutputObserver == nullptr)
		return;

	clog << "FINE: (" << agentName << ") processSend() " << nodeMessageString(nodeMessageType) << " to " << receiver;

	// Next 3 lines = java's wrapper.toByteArray()
	int size = wrapper.ByteSize();
	char* arr = new char[size];
	wrapper.SerializeToArray(arr, size);

	switch(nodeMessageType)
	{
	case NodeMessageType::nodeLeave:
		subscriptions.unsafe_erase(receiver);
		commOutputObserver->processSend(CommunicationMessageType::commLeave, receiver, {});
		break;
	case NodeMessageType::nodeShout:
		commOutputObserver->processSend(CommunicationMessageType::commShout, receiver, arr);
		break;
	case NodeMessageType::nodeWhisper:
		commOutputObserver->processSend(CommunicationMessageType::commWhisper, receiver, arr);
	default:
		throw invalid_argument("unhandled NodeMessageType: " + nodeMessageString(nodeMessageType));
	}
};

void NodeCommunicationLayer::start()
{
	if(commOutputObserver != nullptr)
		commOutputObserver->start();
};

void NodeCommunicationLayer::stop()
{
	execService->cancel();	// do not accept new tasks
	tbb::concurrent_unordered_map<string, pocoTimer::Ptr>::iterator entry;
	for(entry = scheduledTransDataTasks.begin(); entry != scheduledTransDataTasks.end(); entry++)
		entry->second->cancel();
	if(commOutputObserver != nullptr)
		commOutputObserver->stop();
};

bool NodeCommunicationLayer::isConsistentWithPDDL(gp::Map<string, gp::Value> metadata)
{
	bool matched = false;
	set<int> domains = parser.getParserDomain().getDomainIds();
	set<int>::iterator it;
	for(it = domains.begin(); it != domains.end(); it++)
	{
		string domain = parser.getTokenParser().getVariableName(*it);
		ExpressionList metaList;
		gp::Map<string, gp::Value>::iterator itMeta;
		for(itMeta = metadata.begin(); itMeta != metadata.end(); itMeta++)
		{
			string key = itMeta->first;
			gp::Value value = itMeta->second;

			Expression expr;
			expr.setBaseVariable(parser.getTokenParser().getToken(domain + "::" + key));
			expr.setComparator(Equal);

			gp::Value::KindCase kc = value.kind_case();
			switch(kc)
			{
			case value.kNumberValue:
				expr.setValue((float)value.number_value());
				break;
			case value.kBoolValue:
				expr.setBoolean(value.bool_value());
				break;
			case value.kStringValue:
				expr.setToken(parser.getTokenParser().getToken(value.string_value()));
				break;
			case value.kNullValue:
			case value.kStructValue:
			default:
				continue;
			}
			metaList.add(expr);
		}

		vector<ExpressionList> preConditons = agentDefinition.getPreconditions();
		if(preConditons.empty())
			return true;

		vector<ExpressionList>::iterator itList;
		for(itList = preConditons.begin(); itList != preConditons.end(); itList++)
		{
			ExpressionList exprList = *itList;
			if(exprList.containsExpressionList(metaList))
				return true;
		}
	}
	return false;
};

void NodeCommunicationLayer::processCheckCompatibility(string sender, string senderName, string agentDefString)
{
	if(join)
	{
		clog << "INFO: (" << agentName << ") processCheckCompatibilitiy " << senderName;
		list<int> lst = parser.parseString(agentDefString);
		if(!lst.empty())
		{
			int peerAgentId = lst.front();
			clog << "INFO: (" << agentName << ") Checking compatibility with Peer " << senderName;
			// Check if the publisher and subscriber are compatible
			AgentCompatibility agentComp = parser.getParserAgent().getAgent(peerAgentId).checkAgentCompatibility(agentDefinition);

			// If there's any compatibility, request a subscription
			switch(agentComp.getCompatibilityType())
			{
			case AgentCompatibilities::full:
				clog << "INFO: (" << agentName << ") Publisher " << senderName << " is fully compatible";
				requestSubscription(sender);
				break;

			case AgentCompatibilities::sometimes:
				clog << "INFO: (" << agentName << ") Publisher " << senderName << " is sometimes compatible";
				requestSubscription(sender);
				break;

			case AgentCompatibilities::partial:
				clog << "INFO: (" << agentName << ") Publisher " << senderName << " is partially compatible";
				requestSubscription(sender);
				break;

			default:
				clog << "INFO: (" << agentName << ") Publisher " << senderName << " is not compatible";
				break;
			}
		}
		else
			cerr << "WARNING: (" << agentName << ") Check compatibility -- no description returned for peer " << senderName << "!";
	}
};

void NodeCommunicationLayer::processData(NodeMessageType nodeMessageType, string sender, string senderName, string groupName, char data[])
{
	clog << "FINE: (" << agentName << ") processData ()";
	string dataString = data;	// no way to parse an array in C++ protobuf so convert to string
	WrapperMessage wrapper;
	wrapper.ParseFromString(dataString);

	if(wrapper.IsInitialized() && wrapper.has_connectionresponse())
	{
		if(join)
		{
			clog << "INFO: (" << agentName << ") Received connection response from " << senderName;
			processConnectionResponse(wrapper);
		}
	}
	else if(wrapper.IsInitialized() && wrapper.has_transactionoverviewrequest())
	{
		clog << "INFO: (" << agentName << ") Request for transactions from " << senderName;
		setupTransactionRateResponse(sender, wrapper);
	}
	else if(wrapper.IsInitialized() && wrapper.has_connectionrequest())
	{
		clog << "INFO: (" << agentName << ") Received connection request from " << senderName;
		processConnectionRequest(sender, wrapper);
	}
	else if(wrapper.IsInitialized() && wrapper.has_stoprequest())
	{
		clog << "INFO: (" << agentName << ") Received stop request from " << senderName;
		notifyStop(sender, senderName, wrapper);
	}
	else if(wrapper.IsInitialized())
	{
		if(wrapper.has_vector() && !groupName.empty())
		{
			if(isConsistentWithPDDL(wrapper.metadata()))
			{
				clog << "INFO: (" << agentName << ") Received vector message from " << senderName << " and metadata matches PDDL definition";
				notifyReceive(nodeMessageType, sender, senderName, groupName, wrapper);
			}
			else
				clog << "INFO: (" << agentName << ") Received vector message from " << senderName << " but metadata doesn't match PDDL definition. Dropping message.";
		}
		else
			notifyReceive(nodeMessageType, sender, senderName, groupName, wrapper);
	}
};

void NodeCommunicationLayer::requestSubscription(string peerUuid)
{
	// Check if there is an existing subscription to this peer
	tbb::concurrent_unordered_map<string, Subscription>::iterator subIt;
	for(subIt = subscriptions.begin(); subIt != subscriptions.end(); subIt++)
	{
		Subscription sub = subIt->second;
		if(sub.publisher.compare(peerUuid) == 0)
			return;
	}

	// Send request to publisher to subscribe
	ConnectionRequest request;
	request.set_agent_definition(agentDefinition.getDefinition());
	request.set_name(agentName);
	request.set_uuid(identity);

	WrapperMessage wrapper;
	wrapper.add_timeinmicroseconds(chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count());
	wrapper.add_sequence(sequence.operator--());
	wrapper.set_allocated_connectionrequest(&request);

	// Next 3 lines = java's wrapper.toByteArray()
	int size = wrapper.ByteSize();
	char* arr = new char[size];
	wrapper.SerializeToArray(arr, size);

	notifySend(CommunicationMessageType::commWhisper, peerUuid, arr);
	clog << "INFO: (" << agentName << ") Requesting subscription from " << peerUuid;
};

void NodeCommunicationLayer::processConnectionRequest(string sender, WrapperMessage wrapper)
{
	ConnectionRequest request = wrapper.connectionrequest();
	string peerUuid = sender;
	AgentCompatibility checkCompat = checkGoodSubscriber(peerUuid, request.agent_definition());

	if(checkCompat.getCompatibilityType() != AgentCompatibilities::none)
	{
		switch(checkCompat.getCompatibilityType())
		{
		case AgentCompatibilities::full:
			clog << "INFO: (" << agentName << ") Subscriber " << peerUuid << " is fully compatible";
			addSubscriber(peerUuid, checkCompat.getConditions());
			break;

		case AgentCompatibilities::sometimes:
			clog << "INFO: (" << agentName << ") Subscriber " << peerUuid << " is sometimes compatible";
			addSubscriber(peerUuid, checkCompat.getConditions());
			break;

		case AgentCompatibilities::partial:
			clog << "INFO: (" << agentName << ") Subscriber " << peerUuid << " is partially compatible";
			if(confirmPartial(checkCompat.getConditions().at(0)))
				addSubscriber(peerUuid, checkCompat.getConditions());
			break;

		default:	// should never occur
			clog << "INFO: (" << agentName << ") Subscriber " << peerUuid << " is not compatible";
			break;
		}
	}
};

void NodeCommunicationLayer::processConnectionResponse(WrapperMessage wrapper)
{
	ConnectionResponse response = wrapper.connectionresponse();
	string peerUuid = response.uuid();
	clog << "INFO: Subscribed to " << peerUuid;
	string group = response.group();
	addSubscription(group, peerUuid);
};

void NodeCommunicationLayer::setupTransactionRateResponse(string sender, WrapperMessage wrapper)
{
	long rate = wrapper.transactionoverviewrequest().rateinseconds();
	Poco::Util::Timer pTimer;
	pTimer.scheduleAtFixedRate(execService, rate, rate);
	scheduledTransDataTasks.emplace(sender, execService);
};

void NodeCommunicationLayer::sendTransactionSummary(string recipient)
{
	TransactionOverviewResponse c;
	c.set_uuid(identity);
	c.set_name(identity);
	c.set_taskuuid(taskuuid);

	map<string, int> transCounts;
	tbb::concurrent_unordered_map<std::string, Subscription>::iterator s;
	for(s = subscriptions.begin(); s != subscriptions.end(); s++)
	{
		TransactionSummaryItem tsi;
		tsi.set_uuid(s->second.publisher);
		tsi.set_name(s->second.publisher);
		c.add_recvlist();
		transCounts.emplace(s->second.publisher, transCounts.at(s->second.publisher) + s->second.messageCount.load());
	}

	map<string, int>::iterator item;
	for(item = transCounts.begin(); item != transCounts.end(); item++)
	{
		TransactionSummaryItem tsi;
		tsi.set_uuid(item->first);
		tsi.set_name(item->first);
		tsi.set_count(item->second);
		c.add_transactionsummary();
	}

	tbb::concurrent_unordered_map<std::string, Publication>::iterator p;
	for(p = publications.begin(); p != publications.end(); p++)
	{
		for(string sub : p->second.subscribers)
		{
			if(p->second.subscribers.count(sub) == 0)
			{
				p->second.subscribers.emplace(sub);
				TransactionSummaryItem tsi;
				tsi.set_uuid(sub);
				tsi.set_name(sub);
				c.add_sendlist();
			}
		}
	}

	chrono::system_clock::duration dur = chrono::system_clock::now().time_since_epoch();
	WrapperMessage wrapped;
	wrapped.add_timeinmicroseconds(dur.count() * 1000);
	wrapped.add_sequence(sequence.operator--());
	wrapped.set_allocated_transactionoverviewresponse(&c);

	// Next 3 lines = java's wrapper.toByteArray()
	int size = wrapped.ByteSize();
	char* arr = new char[size];
	wrapped.SerializeToArray(arr, size);

	notifySend(CommunicationMessageType::commWhisper, recipient, arr);
};

AgentCompatibility NodeCommunicationLayer::checkGoodSubscriber(string peerUuid, string peerAgentDef)
{
	// Check if agent definition matches published
	list<int> retList = parser.parseString(peerAgentDef);
	if(!retList.empty())
	{
		// There is large block of commented out code in the java file that I am not translating

		int peerAgentId = retList.front();
		return agentDefinition.checkAgentCompatibility(parser.getParserAgent().getAgent(peerAgentId));
	}
	return {};	// return null
};

void NodeCommunicationLayer::addSubscriber(string peerUuid, vector<ExpressionList> conditionList)
{
	for(ExpressionList conditions : conditionList)
	{
		Publication publication = getExpressionListEqual(conditions);
		long millis = chrono::system_clock::now().time_since_epoch().count() * 1000;
		if(publication.group.empty())
		{
			string groupStr = identity + "." + to_string(millis);
			publication = Publication(groupStr, conditions, millis);
			publications.emplace(groupStr, publication);
			clog << "INFO: (" << agentName << ") Creating Group " << publication.group;
		}

		clog << "INFO: Adding subscriber: " << peerUuid;
		publication.subscribers.emplace(peerUuid);
		
		ConnectionResponse response;
		response.set_group(publication.group);
		response.set_uuid(identity);

		WrapperMessage wrapper;
		wrapper.add_timeinmicroseconds(millis);
		wrapper.add_sequence(sequence.operator--());
		wrapper.set_allocated_connectionresponse(&response);

		// Next 3 lines = java's wrapper.toByteArray()
		int size = wrapper.ByteSize();
		char* arr = new char[size];
		wrapper.SerializeToArray(arr, size);
		notifySend(CommunicationMessageType::commWhisper, peerUuid, arr);
		clog << "INFO: (" << agentName << ") Sending connection response to " << peerUuid;
	}
};

Publication NodeCommunicationLayer::getExpressionListEqual(ExpressionList expressionList)
{
	tbb::concurrent_unordered_map<std::string, Publication>::iterator pub;
	for(pub = publications.begin(); pub != publications.end(); pub++)
		if(pub->second.conditions.equalExpressionLists(expressionList))
			return pub->second;
	ExpressionList emptyList;
	return Publication("", emptyList, 0);	// return null
};

void NodeCommunicationLayer::addSubscription(string group, string peerUuid)
{
	if(subscriptions.count(group) == 0)
	{
		long millis = chrono::system_clock::now().time_since_epoch().count() * 1000;
		Subscription sub = Subscription(peerUuid, group, millis);
		subscriptions.emplace(group, sub);
		clog << "INFO: (" << agentName << ") Joining group " << group;
		char* cstr = new char[group.length() + 1];
		strcpy(cstr, group.c_str());
		notifySend(CommunicationMessageType::commJoin, "", cstr);
	}
};

bool NodeCommunicationLayer::confirmPartial(ExpressionList expressionList)
{
	return ((nodeInputObserver != nullptr) ? nodeInputObserver->confirmPartial(expressionList) : false);
};

void NodeCommunicationLayer::processTaskUuid(string uuid)
{
	taskuuid = uuid;
	if(nodeInputObserver != nullptr)
		nodeInputObserver->processTaskUuid(uuid);
};