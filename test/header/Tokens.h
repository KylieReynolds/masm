#define _CRT_SECURE_NO_WARNINGS
#ifndef TOKENS_H
#define TOKENS_H

#include <string>
#include <unordered_map>
#include <fstream>
#include <iostream>

enum TokenValues
{
    NA = 1,
    Define,
    Agent,
    Domain,
    Input,
    Output,
    PreCondition,
    PostCondition,
    Effect,
    And,
    Or,
    Not,
    OpenParen,
    CloseParen,
    GreaterThan,
    LessThan,
    Equal,
    GE,
    LE,
    NE,
    VariableSeparator,
    Dash,
    True,
    False,
    Requirements,
    Types,
    Constants,
    Predicates,
    Strips,
    Typing,
    Object,
    Number,
    SString,
    Null,
    START,
    RangeGTLT,
    RangeGELE,
    RangeGTLE,
    RangeGELT,
    Parameters,
    Functions,
    Fluent,
    Increase,
    Decrease,
    Assign,
    Boolean
};

enum TokenOccurence
{
    occurNever = 0,
    occurAlways,
    occurSometimes
};

class Tokens
{
public:
    friend class NodeCommunicationLayer;
    const int firstVariable = 100;

protected:
    std::unordered_map<std::string, int> variables;
    std::unordered_map<int, std::string> variablesRev;
    int nextVariable = firstVariable;

private:
    int nextToken;
    int lookahead;
    int currentPos;
    std::ifstream inputStream;
    std::string inputString;

public:
    Tokens();
    Tokens(const Tokens& t);  // Copy constructor
    Tokens& operator=(const Tokens& t);  // Assignment operator
    static std::string getName(int token);
    static int getTokenHelper(std::string s);

protected:
    static bool isSymbol(int c);
    static bool isComparator(int c);
    static bool isWhitespace(int c);

    void setLookahead(int look) { lookahead = look; };
    void setCurrentPos(int currPos) { currentPos = currPos; };
    void setNextToken(int token) { nextToken = token; };
    void setInputString(std::string input) { inputString = input; };
    void setInputStream(std::ifstream& stream) { inputStream.swap(stream); };
    std::string getInputString() { return inputString; };
    std::string getVariableName(int token);
    // std::ifstream getInputStream() { return inputStream.open(); };
    
    bool isPredefined(int token) { return ( token < firstVariable ); };
    bool isVariable(int token) { return !isPredefined(token); };
    bool isNumber(int c);

    int getToken(std::string s);
    int getNextChar();
    std::string getNextWord();
    int getNextToken();
    int tokenize(std::string s);

public:
    std::unordered_map<int, std::string> getDictionary() { return variablesRev; };
    int nextToke() { return nextToken; };
};

#endif