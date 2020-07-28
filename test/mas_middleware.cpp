// mas_middleware.cpp : Defines the entry point for the application.

#include "header/AgentFactory.h"
#include "header/HitmanAgent.h"
#include "header/SpyAgent.h"
#include "header/Mas.h"
#include "src/AgentUtil.cpp"

using namespace std;

class middleware : public HitmanAgent, public SpyAgent
{
	int main(vector<string> args)
	{
		AgentUtil au;
		/**********		HITMAN AGENT MAIN	 **********/
		ArgumentParser h_argumentParser;
		h_argumentParser.parse_arguments(args);
		
		// add default domain definition file if not provided
		if(!h_argumentParser.hasArgument("domain-definition-file"))
			h_argumentParser.addArgument("--domain-definition-file", au.all_domain_definitions_file);

		if(!h_argumentParser.hasArgument("agent-definition-string") && !h_argumentParser.hasArgument("agent-definition-file"))
			h_argumentParser.addArgument("--agent-definition-string", h_agent_definition);

		h_argumentParser.addArgument("--agent-name", h_default_agent_name);
		if(h_argumentParser.hasArgument("peer-uuid"))
		{
			try
			{
				HitmanAgent h_agent = HitmanAgent (AgentFactory::getAgent(typeid(h_agent).name(), h_argumentParser));
				h_agent.start();
				for(string uuid : h_argumentParser.getArgumentList("peer-uuid"))
					if(!uuid.empty())
						h_agent.sendStopRequest(uuid);
				h_agent.stop();
			}
			catch(bad_typeid err)
			{
				cerr << "SEVERE: " << err.what();
			}
		}

		/**********		SPY AGENT MAIN		 **********/
		ArgumentParser s_argumentParser;
		s_argumentParser.parse_arguments(args);
		
		// add default domain definition file if not provided
		if (!s_argumentParser.hasArgument("domain-definition-file"))
			s_argumentParser.addArgument("--domain-definition-file", au.all_domain_definitions_file);

		s_argumentParser.addArgument("--agent-definition-string", s_agent_definition);
		s_argumentParser.addArgument("--agent-name", s_default_agent_name);

		try
		{
			SpyAgent s_agent = SpyAgent (AgentFactory::getAgent(typeid(s_agent).name(), s_argumentParser));
			s_agent.start();
			int found = s_agent.doQuery();
			while (found == 0)
				found = s_agent.doQuery();
			clog << "There were " << found << " agents found.";
			s_agent.stop();
		}
		catch (bad_typeid err)
		{
			cerr << "SEVERE: " << err.what();
		}

		return 0;
	}

	/**********		MAS MAIN		 **********/
	// must be rewritten to parse options from a JSON file. Part of rearchitecture so translation not needed
	// java code is:
	/****
		String port = "";
        Options options = new Options();
        Option input = new Option("i", "input", true, "Starts agents listed by agents xml.");
        options.addOption(input);
        Option stopagent = new Option("d","destroy.", true, "Stop this agent by uuid.");
        options.addOption(stopagent);
        Option listagent = new Option("l", "listAgent", false, "List all running agents uuid on system.");
        options.addOption(listagent);
        Option portnum = new Option("p", "port", true, "(Optional) Port to run for stopping or listening on.");
        options.addOption(portnum);

        CommandLineParser parser = new DefaultParser();
        HelpFormatter formatter = new HelpFormatter();
        CommandLine cmd;

        try {
            cmd = parser.parse(options, args);
            if (cmd.hasOption(portnum.getOpt())){
                port = cmd.getOptionValue(portnum.getOpt());
            }
            if (cmd.hasOption(input.getOpt())){
                parseAgentXml(cmd.getOptionValue(input.getOpt()));
            }else if(cmd.hasOption(stopagent.getOpt())){
                stopAgent(cmd.getOptionValue(stopagent.getOpt()),port);
            }else if(cmd.hasOption(listagent.getOpt())){
                listenAgents(port);
            }else{
                formatter.printHelp("java -jar act3-mas-launcher-<version>.jar ",options);
            }

        } catch (ParseException e) {
            logger.log(Level.SEVERE, e.toString());
            System.exit(-1);
        }
	****/
};
