#ifndef PDDLPARSERDOMAIN_H
#define PDDLPARSERDOMAIN_H

#include "Tokens.h"
#include <list>
#include <map>
#include <set>

class PredicateDefinition
{
public:
    std::map<int, int> variables;
    PredicateDefinition() { variables; };
    void put(int key, int value) { variables.emplace(key, value); };
    int get(int key) { return variables.at(key); };
    int size() { return variables.size(); };
};

class DomainDefinition
{
public:
    std::string name;
    std::list<int> requirements;
    std::map<int, int> types;
    std::map<int, int> constants;
    std::map<int, PredicateDefinition> predicates;
    std::map<int, PredicateDefinition> functions;
    std::map<int, PredicateDefinition> getPredicates();
};

class PDDLParserDomain : public Tokens
{
private:
    static const int PREDICATE = 1;
    static const int FUNCTION = 2;
    std::map<int, DomainDefinition> domains;
    Tokens tokenParser;

public:
    PDDLParserDomain() {};  // Default constructor
    PDDLParserDomain(Tokens aTokenParser) { tokenParser = aTokenParser; };

    DomainDefinition getDomain(int domainId);
    std::set<int> getDomainIds();

private:
    PredicateDefinition getDefinitionHelper(int token, int type);
    int getDomainCountHelper(int predicate, int type);
    int getDomainVariableHelper(int predicate, int type);
    int getDomainPositionHelper(int predicate, int type);

public:
    PredicateDefinition getPredicates(int token) { return getDefinitionHelper(token, PREDICATE); };
    PredicateDefinition getFunctions(int token) { return getDefinitionHelper(token, FUNCTION); };
    int getPredicateDomainCount(int pred) { return getDomainCountHelper(pred, PREDICATE); };
    int getFunctionDomainCount(int func) { return getDomainCountHelper(func, FUNCTION); };
    int getPredicateDomainVariable(int pred) { return getDomainVariableHelper(pred, PREDICATE); };
    int getFunctionDomainVariable(int func) { return getDomainVariableHelper(func, FUNCTION); };
    int getPredicateDomainPosition(int pred) { return getDomainPositionHelper(pred, PREDICATE); };
    int getFunctionDomainPosition(int func) { return getDomainPositionHelper(func, FUNCTION); };
    bool isPredicate(int token) { return getDefinitionHelper(token, PREDICATE).size() > 0; };
    bool isFunction(int token) { return getDefinitionHelper(token, FUNCTION).size() > 0; };

    int getPredicateType(int domain, int predicate);
    int getPredicateType(int domain, int predicate, int variable);
    std::map<std::string, std::string> getConstants(int domain);
    bool printDomain(int domainId);
    void printDomains();
    int parseDomain();
};

#endif