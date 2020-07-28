#ifndef AGENTUTIL_H
#define AGENTUTIL_H
/**********
 * Java program finds the default root path and definition file using system properties.
 * C++ cannot do this cross platform so for the classes that need to call AgentUtils, the filePath
 * should be given explicitly.
 **********/

#include <string>
#include <poco/Path.h>

class AgentUtil
{
public:
	const std::string default_pddl_root_path = Poco::Path::current() + "/pddl_files/";
	const std::string all_domain_definitions_file = default_pddl_root_path + "AllDomainDefinitions.pddl";

	// Reads the definition file into a string
	static std::string readDefinitionFile(std::string filePath);
};

#endif