#ifndef ARGUMENTPARSER_H
#define ARGUMENTPARSER_H

#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

class ArgumentParser    // Utility class for parsing arguments
{
private:
    std::unordered_map<std::string, std::list<std::string>> arguments;  // unordered_map of command line arguments
    std::vector<std::string> args;  // vector of command line arguments

public:
    ArgumentParser() {};    // default constructor
    ArgumentParser(std::vector<std::string> argues);

    const void parse_arguments(const std::vector<std::string> argues);  // Parse arguments and stores in map
    bool hasArgument(const std::string argumentKey);
    std::string getArgumentOrDefault(const std::string argumentKey, const std::string value);   // get first value of argument
    std::string getArgument(const std::string argumentKey);     // get first value for argument
    const std::list<std::string> getArgumentList(const std::string argumentKey); // get list of values for argument
    void addArgument(const std::string param, const std::string value); // add argument to list 
    std::vector<std::string> getArgs() { return args; };
};

#endif