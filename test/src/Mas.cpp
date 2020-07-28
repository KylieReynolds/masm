#include "../header/Mas.h"
#include "AgentUtil.cpp"

#include <map>
#include <poco/File.h>
#include <poco/Path.h>
#include <poco/String.h>

#include <poco/DOM/Document.h>
#include <poco/DOM/DOMParser.h>
#include <poco/DOM/NodeList.h>
#include <poco/SAX/InputSource.h>
#include <poco/Util/AbstractConfiguration.h>
#include <poco/Util/PropertyFileConfiguration.h>

using namespace std;
namespace px = Poco::XML;

void Mas::parseAgentXml(string filename)
{
	map<string, map<string, string>> agentList;
	map<string, string> environmentList;
	list<string> argList;
	try
	{
		px::InputSource propertyFile(filename);
		px::DOMParser dParser;
		px::Document* doc = dParser.parse(&propertyFile);
		px::NodeList* agent = doc->getElementsByTagName("agent");

		// for agents to load
		for(int i = 0; i < agent->length(); i++)
		{
			px::Node* nd = agent->item(i);
			if(nd->nodeType() == px::Node::ELEMENT_NODE)
			{
				Poco::Util::AbstractConfiguration* abConf;
				px::Element* el = (px::Element*) nd;
				string name = "";
				
				//pull agent or class name
				px::NodeList* nm = el->getElementsByTagName("name");
				if(nm->length() > 0)
					name = Poco::trim(nm->item(0)->getNodeValue()); // string without leading/trailing whitespace

				// read parameter line properties
				nm = el->getElementsByTagName("propertiesfile");
				for(int j = 0; j < nm->length(); j++)
				{
					string propfile = Poco::trim(nm->item(j)->getNodeValue());
					// append multiple properties per agent
					if(agentList.find(name) != agentList.end())
					{
						map<string, string> aproperty = agentList.at(name);
						map<string, string> itMap = loadProperties(propfile);
						for(auto it : itMap)
							aproperty.emplace(it.first, it.second);
						agentList.emplace(name, aproperty);
					}
					else
						agentList.emplace(name, loadProperties(propfile));
				}

				// handle command line flags that have pairs that do not fit into property file
				nm = el->getElementsByTagName("cmd");
				for(int t = 0; t < nm->length(); t++)
				{
					string tag = Poco::trim(nm->item(t)->getNodeValue());
					argList.push_back(tag);
				}

				// load environment settings
				nm = el->getElementsByTagName("environmentfile");
				for(int e = 0; e < nm->length(); e++)
				{
					string envfile = Poco::trim(nm->item(e)->getNodeValue());
					map<string, string> itMap = loadProperties(envfile);
					for(auto it : itMap)
						environmentList.emplace(it.first, it.second);
				}
			}
		}
	}
	catch(exception e)
	{
		cerr << "SEVERE: " << e.what();
	}

	for(auto entry: agentList)
	{
		string agentname = entry.first;
		map<string, string> paramMap = entry.second;
		list<string> cmdList;

		list<string> paramList = convertParameterList(paramMap);
		for(auto param : paramList)
			cmdList.push_back(param);

		for(auto arg : argList)
			cmdList.push_back(arg);

		clog << "INFO: running agent " + agentname;
		startAgent(agentname, environmentList, cmdList);
	}
};

map<string, string> Mas::loadProperties(string filename)
{
	map<string, string> params;
	ifstream input(filename);
	Poco::Util::PropertyFileConfiguration* prop = new Poco::Util::PropertyFileConfiguration(input);
	Poco::Path p(filename);
	Poco::File f(p);
	if(!f.exists())
		return params;

	try
	{
		vector<string> keyList;
		prop->load(input);
		clog << "INFO: loaded " << filename;
		string str;
		while(getline(input, str))
		{
			int pos = static_cast<int>(str.find(":"));
			string key = str.substr(0, pos);
			string value = str.substr(pos + 1);
			params.emplace(key, value);
		}
	}
	catch(ios_base::failure fail)
	{
		cerr << "SEVERE: " << fail.what();
	}
	if(input.peek() != EOF)
	{
		try
		{
			input.close();
		}
		catch(ios_base::failure fail)
		{
			cerr << "SEVERE: " << fail.what();
		}
	}
	return params;
};

void Mas::stopAgent(string uuid, string port)
{
	ArgumentParser ap;
	HitmanAgent ha;
	AgentUtil au;
	ap.addArgument("--agent-definition-string", ha.h_agent_definition);
	ap.addArgument("--agent-name", ha.h_default_agent_name);
	ap.addArgument("--domain-definition-file", au.all_domain_definitions_file);
	if(port.length() != 0)
		ap.addArgument("--pyre-port", port);
	try
	{
		HitmanAgent agent007 = HitmanAgent(AgentFactory::getAgent(typeid(ha).name(), ap));
		agent007.start();
		agent007.sendStopRequest(uuid);
		agent007.stop();
	}
	catch(bad_typeid err)
	{
		cerr << "SEVERE: Class not found for HitmanAgent! " << err.what();
	}
};

void Mas::startAgent(string agentname, map<string, string> environmenthash, list<string> args)
{
	try
	{
		AgentSpawner spawner;
		list<string> cmdln;
		if(agentname.find(".py") != string::npos)
		{
			cmdln.push_back("python");
			Poco::Path p(agentname);
			agentname = p.getFileName();
		}
		else
			cmdln.push_back("java");	// Change to cpp??
		cmdln.push_back(agentname);
		for(auto x : args)
			cmdln.push_back(x);
		spawner.spawnAgent(cmdln, convertParameterList(environmenthash), agentname);
	}
	catch(invalid_argument err)
	{
		cerr << "SEVERE: " << err.what();
	}
};

list<string> Mas::listenAgents(string port)
{
	list<string> results;

	ArgumentParser ap;
	SpyAgent sa;
	AgentUtil au;
	ap.addArgument("--agent-definition-string", sa.s_agent_definition);
	ap.addArgument("--agent-name", sa.s_default_agent_name);
	ap.addArgument("--domain-definition-file", au.all_domain_definitions_file);
	if(port.length() != 0)
		ap.addArgument("--pyre-port", port);
	try
	{
		SpyAgent felix = SpyAgent(AgentFactory::getAgent(typeid(sa).name(), ap));
		felix.start();
		int size = felix.doQuery();
		clog << "INFO: Found " << size << " agents running.";
		felix.stop();
	}
	catch(bad_typeid err)
	{
		cerr << "SEVERE: Class not found for SpyAgent! " << err.what();
	}
	return results;
}

vector<string> Mas::convertParameterHash(map<string, string> params)
{
	vector<string> retVect;
	list<string> temp = convertParameterList(params);
	for(auto x : temp)
		retVect.push_back(x);
	return retVect;
}

list<string> Mas::convertParameterList(map<string, string> params)
{
	list<string> args;
	for(auto x : params)
	{
		args.push_back("--" + x.first);
		args.push_back(x.second);
	}
	return args;
};