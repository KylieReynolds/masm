#include "../header/SpyAgent.h"
using namespace std;

SpyAgent::SpyAgent(ArgumentParser argumentParser)
{
	spyArgParser = ArgumentParser::ArgumentParser(argumentParser);	// similar to Java's super keyword usage
};
SpyAgent::SpyAgent(const AgentBase& ab)
{
	spyArgParser = AgentBase::getArgumentParser();
}

void SpyAgent::processPeerArrived(string peer, string peerName)
{
	peers.emplace(peer, peerName);
	clog << "INFO: Found name " << peerName << ", UUID " << peer;
};

void SpyAgent::processPeerDeparted(string peer, string peerName)
{
	peers.unsafe_erase(peer);
};

int SpyAgent::doQuery()
{
	try
	{
		waitForUuid();
	}
	catch (exception e)
	{
		cerr << "WARNING: interrupted waiting for UUID..." << e.what();
	}

	int size = peers.size();
	tbb::concurrent_unordered_map<string, string>::iterator it;
	for(it = peers.begin(); it != peers.end(); it++)
		clog << "INFO: Name: " << it->first << ", UUID: " << it->second;
	return size;
};