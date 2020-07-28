#include "..\header\ArgumentParser.h"
#include <poco/String.h>

using namespace std;

ArgumentParser::ArgumentParser(vector<string> argues)
{
    parse_arguments(argues);
};

const void ArgumentParser::parse_arguments(const vector<string> argues)
{
    args = argues;
    // parse arguments; expecting --port <port> --ipaddress <ipaddress>
    if(args.size() > 0)
    {
        for(int i = 0; i < args.size(); i++)
        {
            if(args.at(i).length() > 1 && args.at(i).compare(0, 2, "--") == 0)
            {
                if(args.size() == i)
                    break;
                else if(args.size() > i + 1.0)
                {
                    if(args.at(i + 1.0).length() > 1 && args.at(i + 1.0).compare(0, 2, "--") == 0)
                        addArgument(args.at(i), "true");
                    else
                        addArgument(args.at(i), args.at(i + 1.0));
                }
                else
                    addArgument(args.at(i), "true");
            }
        }
    }
};

bool ArgumentParser::hasArgument(const string argumentKey)
{
    return arguments.find(argumentKey) != arguments.end();
};

string ArgumentParser::getArgumentOrDefault(const string argumentKey, const string value)
{
    if(hasArgument(argumentKey))
    {
        list<string> temp = arguments.at(argumentKey);
        if(temp.size() > 1)
            cout << "Warning: Only Return the first argument in the list";
        return temp.front();
    }
    else
        return value;
};

string ArgumentParser::getArgument(const string argumentKey)
{
    if(!hasArgument(argumentKey))
        throw invalid_argument("No " + argumentKey + " specified (use --" + argumentKey + " <" + argumentKey + ">)");
    list<string> temp = arguments.at(argumentKey);
    if(temp.size() > 1)
        cout << "Warning: Only Return the first argument in the list";
    return temp.front();
};

const list<string> ArgumentParser::getArgumentList(const string argumentKey)
{
    if(!hasArgument(argumentKey))
        throw invalid_argument("No " + argumentKey + " specified (use --" + argumentKey + " <" + argumentKey + ">)");
    return arguments.at(argumentKey); // to be unmodifiable the return type of this method is a constant
};

void ArgumentParser::addArgument(const string param, const string value)
{
    string parameter = Poco::toLower(param.substr(2));

    //make sure item does not already exist
    if (arguments.find(parameter) == arguments.end())
    {
        // arguments.emplace(parameter, new list<string>);
        vector<string> tmp(args.size() + 2);
        for (int i = 0; i < args.size(); i++)
            tmp.at(i) = args.at(i);
        tmp.at(args.size()) = param;
        tmp.at(args.size() + 1) = value;
        args = tmp;
    }

    arguments.at(parameter).push_back(value);
};