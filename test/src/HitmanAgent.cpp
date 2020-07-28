#include "../header/HitmanAgent.h"
#include "Act3Formatter.cpp"

#include <chrono>
#include <protos/Communication.pb.h>
using namespace mas_middleware::protos;
using namespace std;

HitmanAgent::HitmanAgent(ArgumentParser argumentParser)
{
	hitmanArgParse = ArgumentParser::ArgumentParser(argumentParser);	// similar to Java's super keyword usage
};
HitmanAgent::HitmanAgent(const AgentBase& ab)
{
	hitmanArgParse = AgentBase::getArgumentParser();
}

void HitmanAgent::sendStopRequest(string uuid)
{
	if(uuid.empty())
		return;

	try
	{
		waitForUuid();
	}
	catch(exception e)
	{
		cerr << "SEVERE: interrupted waiting for UUID..." << e.what();
	}

	// Send a subscription request to the publisher
	StopRequest request;
	request.set_name(agentName);
	request.set_uuid(myIdentity);
	WrapperMessage wrapper;
	wrapper.add_timeinmicroseconds(chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count());
	wrapper.add_sequence(sequence.operator++());
	wrapper.set_allocated_stoprequest(&request);

	clog << "INFO: Sending stop request to " << uuid;
	sendMessage(uuid, wrapper);
};