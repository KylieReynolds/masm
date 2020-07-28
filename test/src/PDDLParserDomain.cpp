#include "..\header\PDDLParserDomain.h"
#include <sstream>

using namespace std;

map<int, PredicateDefinition> DomainDefinition::getPredicates()
{
    if (!predicates.empty())
        return map<int, PredicateDefinition>(predicates);
    else
        return {}; // {} is the equivalent to java's null map
};

DomainDefinition PDDLParserDomain::getDomain(int domainId) 
{
    if (domains.find(domainId) != domains.end())
        return domains.at(domainId);
    else
        return {};
};

set<int> PDDLParserDomain::getDomainIds()
{
    map<int, DomainDefinition>::iterator it;
    set<int> domSet;
   for(it = domains.begin(); it != domains.end(); it++)
        domSet.insert(it->first);
    return domSet;
};

PredicateDefinition PDDLParserDomain::getDefinitionHelper(int token, int type)
{
    map<int, DomainDefinition>::iterator it;
    for (it = domains.begin(); it != domains.end(); it++)
    {
        DomainDefinition domDef = it->second;
        PredicateDefinition predDef;
        if (type == PREDICATE)
            predDef = domDef.predicates.at(token);
        else if (type == FUNCTION)
            predDef = domDef.functions.at(token);

        if (predDef.size() > 0) // if not null
            return predDef;
    }
    return {}; // return null
};

int PDDLParserDomain::getDomainCountHelper(int predicate, int type)
{
    PredicateDefinition predDef = getDefinitionHelper(predicate, type);
    if (predDef.size() == 0)
        return 0;

    int count = 0;
    map<int, int> pVar = predDef.variables;
    map<int, int>::iterator itVars;
    for (itVars = pVar.begin(); itVars != pVar.end(); itVars++)
    {
        int variable = itVars->first;             // get key
        int atype = itVars->second;               // get value
        if (domains.find(atype) != domains.end()) // if not null
            count++;
    }
    return count;
};

int PDDLParserDomain::getDomainVariableHelper(int predicate, int type)
{
    PredicateDefinition predDef = getDefinitionHelper(predicate, type);
    if (predDef.size() == 0)
        return 0;

    map<int, int> pVar = predDef.variables;
    map<int, int>::iterator itVars;
    for(itVars = pVar.begin(); itVars != pVar.end(); itVars++)
    {
        int variable = itVars->first;             // get key
        int atype = itVars->second;               // get value
        if (domains.find(atype) != domains.end()) // if not null
            return variable;
    }
    return NA;
};

int PDDLParserDomain::getDomainPositionHelper(int predicate, int type)
{
    PredicateDefinition predDef = getDefinitionHelper(predicate, type);
    if (predDef.size() == 0)
        return -1;

    int count = 0;
    map<int, int> pVar = predDef.variables;
    map<int, int>::iterator itVars;
    for(itVars = pVar.begin(); itVars != pVar.end(); itVars++)
    {
        int variable = itVars->first;             // get key
        int atype = itVars->second;               // get value
        if (domains.find(atype) != domains.end()) // if not null
            return count;
        count++;
    }
    return -1;
};

int PDDLParserDomain::getPredicateType(int domain, int predicate)
{
    DomainDefinition domDef = domains.at(domain);
    PredicateDefinition predDef = domDef.predicates.at(predicate);
    map<int, int> pVar = predDef.variables;

    if (predDef.size() == 0 || pVar.begin() == pVar.end())
        return NA;

    map<int, int>::iterator itVars;
    for(itVars = pVar.begin(); itVars != pVar.end(); itVars++)
    {
        int chkVar = itVars->first;                 // get key
        int retType = itVars->second;               // get value
        if (domains.find(retType) != domains.end()) // if not null
            continue;
        
        if (isPredefined(retType))
            return retType;
        else if (domDef.types.find(retType) == domDef.types.end()) // if null
            return NA;
        else
            return domDef.types.at(retType);
    }
    return Boolean;
};

int PDDLParserDomain::getPredicateType(int domain, int predicate, int variable)
{
    DomainDefinition domDef = domains.at(domain);
    PredicateDefinition predDef = domDef.predicates.at(predicate);
    if (predDef.size() == 0)
        return NA;

    map<int, int> pVar = predDef.variables;
    map<int, int>::iterator itVars;
    for(itVars = pVar.begin(); itVars != pVar.end(); itVars++)
    {
        int chkVar = itVars->first;   // get key
        int retType = itVars->second; // get value
        if (chkVar == variable)
        {
            if (isPredefined(retType))
                return retType;
            else if (domDef.types.find(retType) == domDef.types.end()) // if null
                return NA;
            else
                return domDef.types.at(retType);
        }
    }
    return NA;
};

map<string, string> PDDLParserDomain::getConstants(int domain)
{
    map<string, string> retMap;
    DomainDefinition domDef = domains.at(domain);
    map<int, int> dConst = domDef.constants;
    map<int, int>::iterator it;
    for(it = dConst.begin(); it != dConst.end(); it++)
    {
        string key = getVariableName(it->first);
        string value = getVariableName(it->second);
        retMap.emplace(key, value);
    }
    return retMap;
};

bool PDDLParserDomain::printDomain(int domainId)
{
    if (domains.find(domainId) == domains.end())
        return false;
    DomainDefinition domDef = domains.at(domainId);

    ostringstream oss;

    oss << "\n\nDomain " << domainId << " - " << domDef.name;
    oss << "\n\tRequirements:\t";
    for (int reqId : domDef.requirements)
        oss << getVariableName(reqId) << '\t';

    oss << "\n\tTypes:\n";
    for (auto it = domDef.types.begin(); it != domDef.types.end(); it++)
        oss << "\t\t" << getVariableName(it->first) << ": " << getVariableName(it->second) << '\n';

    oss << "\n\tConstants:\n";
    for (auto it = domDef.constants.begin(); it != domDef.constants.end(); it++)
        oss << "\t\t" << getVariableName(it->first) << ": " << getVariableName(it->second) << '\n';

    oss << "\n\tFunctions:\n";
    for (auto it = domDef.functions.begin(); it != domDef.functions.end(); it++)
    {
        oss << "\t\t" << getVariableName(it->first) << '\n';
        for (auto it2 = it->second.variables.begin(); it2 != it->second.variables.end(); it++)
            oss << "\t\t\t\t\t" << getVariableName(it2->first) << ": " << getVariableName(it2->second) << '\n';
    }

    oss << "\n\tPredicates:\n";
    for (auto it = domDef.predicates.begin(); it != domDef.predicates.end(); it++)
    {
        oss << "\t\t" << getVariableName(it->first) << '\n';
        for (auto it2 = it->second.variables.begin(); it2 != it->second.variables.end(); it++)
            oss << "\t\t\t\t\t" << getVariableName(it2->first) << ": " << getVariableName(it2->second) << '\n';
    }

    oss << '\n';
    clog << "INFO: " << oss.str();
    return true;
};

void PDDLParserDomain::printDomains()
{
    map<int, DomainDefinition>::iterator it;
    for(it = domains.begin(); it != domains.end(); it++)
        printDomain(it->first);
};

int PDDLParserDomain::parseDomain()
{
    string domainName = getNextWord();
    list<int> vars;
    int defType;
    int varId;
    bool atleastOne, atleastOneIn, atleastOneInAgain;

    if (domainName.empty())
    {
        cerr << "WARNING: PDDLParserDomain - domain parsing -- domain name is blank -- returning";
        return -1;
    }

    int domId = tokenize(domainName);
    DomainDefinition domDef;
    if (domains.find(domId) == domains.end())
    {
        domDef.name = domainName;
        domains.emplace(domId, domDef);
    }
    domDef = domains.at(domId);

    if (getNextToken() != CloseParen)
    {
        cerr << "WARNING: PDDLParserDomain - domain parsing -- missing close parenthesis -- returning";
        return -1;
    }

    // iterate through definition
    atleastOne = false;
    getNextToken();
    while (true)
    {
        if (nextToke() != OpenParen) // first token needs to be open parenthesis
        {
            if (!atleastOne)
            {
                cerr << "WARNING: PDDLParserDomain - domain parsing -- missing open parenthesis -- returning";
                return -1;
            }
            else if (nextToke() != CloseParen)
            {
                cerr << "WARNING: PDDLParserDomain - domain parsing -- missing close parenthesis -- returning";
                return -1;
            }
            return domId;
        }

        getNextToken();
        switch (nextToke()) // Get and switch next token
        {
        case Requirements:
            atleastOneIn = false;
            while (true) // iterate through requirements
            {
                getNextToken();
                if (!isPredefined(nextToke()))
                {
                    cerr << "WARNING: PDDLParserDomain - domain parsing -- requirements term not defined -- returning";
                    return -1;
                }
                else if (nextToke() == CloseParen)
                {
                    if (atleastOneIn)
                    {
                        getNextToken();
                        break;
                    }
                    else
                    {
                        cerr << "WARNING: PDDLParserDomain - domain parsing -- empty requirements definition -- returning";
                        return -1;
                    }
                }
                domDef.requirements.emplace_back(nextToke());
                atleastOneIn = true;
            }
            break;

        case Types:
        case Constants:
            defType = nextToke();
            atleastOneIn = false;
            vars.clear();
            getNextToken();
            while (true) // Iterate through types/constants
            {
                if (nextToke() == CloseParen)
                {
                    if (atleastOneIn)
                    {
                        getNextToken();
                        break;
                    }
                    else
                    {
                        cerr << "WARNING: PDDLParserDomain - domain parsing -- empty types/constants definition -- returning";
                        return -1;
                    }
                }
                else if (nextToke() == Dash)
                {
                    if (vars.empty())
                    {
                        cerr << "WARNING: PDDLParserDomain - domain parsing -- empty types definition -- returning";
                        return -1;
                    }

                    // Get type and add variables to the appropriate types/constants
                    getNextToken();
                    list<int>::iterator it;
                    for (it = vars.begin(); it != vars.end(); it++)
                    {
                        varId = *it;
                        if (defType == Types)
                            domDef.types.emplace(varId, nextToke());
                        else
                            domDef.constants.emplace(varId, nextToke());
                    }

                    atleastOneIn = true;
                    vars.clear();
                    getNextToken();
                    continue;
                }
                else if (isPredefined(nextToke()) && nextToke() != SString && nextToke() != Object && nextToke() != Number)
                {
                    cerr << "WARNING: PDDLParserDomain - domain parsing -- type {0} is a reserved word ({1}) -- returning";
                    return -1;
                }
                vars.emplace_back(nextToke());
                getNextToken();
            }
            break;

        case Functions:
        case Predicates:
            int whichOne = nextToke();
            atleastOneIn = false;
            getNextToken();

            while (true)
            {
                if (nextToke() != OpenParen)
                {
                    if (atleastOneIn)
                    {
                        cerr << "WARNING: PDDLParserDomain -- domain parsing -- empty predicates definition -- returning";
                        return -1;
                    }
                    else if (nextToke() != CloseParen)
                    {
                        cerr << "WARNING: PDDLParserDomain -- domain parsing -- predicates missing close perenthesis -- returning";
                        return -1;
                    }
                    getNextToken();
                    break;
                }

                // Predicate name is after open parenthesis
                int predicate = getNextToken();
                atleastOneInAgain = false;
                PredicateDefinition varList;
                vars.clear();
                getNextToken();

                // Iterate through predicate information
                while (true)
                {
                    if (nextToke() == CloseParen)
                    {
                        if (atleastOneInAgain)
                        {
                            getNextToken();
                            break;
                        }
                        else
                        {
                            cerr << "WARNING: PDDLParserDomain -- domain parsing -- empty predicates definition -- returning";
                            return -1;
                        }
                    }
                    else if (nextToke() == Dash)
                    {
                        if (vars.empty())
                        {
                            cerr << "WARNING: PDDLParserDomain - domain parsing -- empty predicates definition -- returning";
                            return -1;
                        }

                        getNextToken(); // Get type
                        list<int>::iterator it;
                        for(it = vars.begin(); it != vars.end(); it++)
                            varList.put(*it, nextToke());

                        if (whichOne == Predicates)
                            domDef.predicates.emplace(predicate, varList);
                        else
                            domDef.functions.emplace(predicate, varList);

                        atleastOneInAgain = true;
                        atleastOneIn = true;
                        vars.clear();
                        getNextToken();
                        continue;
                    }

                    if (isPredefined(nextToke()))
                    {
                        cerr << "WARNING: PDDLParserDomain - domain parsing -- type " << nextToke() << " is a reserved word (" << getVariableName(nextToke()) << ") -- returning";
                        return -1;
                    }

                    vars.emplace_back(nextToke());
                    getNextToken();
                }
            }
            break;
        }

        atleastOne = true;
    }
};