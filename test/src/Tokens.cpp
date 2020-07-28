#include "..\header\Tokens.h"
#include <algorithm>
#include <ostream>
#include <poco/String.h>

using namespace std;

Tokens::Tokens()
{
    variables;
    variablesRev;
    nextVariable = firstVariable;
    lookahead = -2;
    currentPos = -1;
    inputStream;
    inputString;
    nextToken = NA;
};
Tokens::Tokens(const Tokens& t)  // Copy constructor
{
    variables = t.variables;
    variablesRev = t.variablesRev;
    nextVariable = t.nextVariable;
    lookahead = t.lookahead;
    currentPos = t.currentPos;
    inputStream;
    inputString;
    nextToken = t.nextToken;
};
Tokens& Tokens::operator=(const Tokens& t)  // Assignment operator
{
    variables = t.variables;
    variablesRev = t.variablesRev;
    nextVariable = t.nextVariable;
    lookahead = t.lookahead;
    currentPos = t.currentPos;
    inputStream;
    inputString;
    nextToken = t.nextToken;
    return *this;
};

string Tokens::getName(int token)
{
    switch (token)
    {
    case And:
        return "AND";
    case Or:
        return "OR";
    case Not:
        return "NOT";
    case OpenParen:
        return "Open Paren";
    case CloseParen:
        return "Close Paren";
    case GreaterThan:
        return "GT";
    case LessThan:
        return "LT";
    case Equal:
        return "EQ";
    case GE:
        return "GE";
    case LE:
        return "LE";
    case NE:
        return "NE";
    case True:
        return "True";
    case False:
        return "False";
    case Strips:
        return "Strips";
    case Typing:
        return "Typing";
    case Object:
        return "Object";
    case Number:
        return "Number";
    case SString:
        return "String";
    case RangeGTLT:
        return " (RANGE) ";
    case RangeGELE:
        return " [RANGE] ";
    case RangeGTLE:
        return " (RANGE] ";
    case RangeGELT:
        return " [RANGE) ";
    case Fluent:
        return "Fluents";
    case Increase:
        return "Increase";
    case Decrease:
        return "Decrease";
    case Assign:
        return "Assign";
    default:
        return "";
    }
}

int Tokens::getTokenHelper(string s)
{
    Poco::toLowerInPlace(s);
    if (s == "true")
        return True;
    else if (s == "false")
        return False;
    else if (s == "define")
        return Define;
    else if (s == "agent" || s == "action")
        return Agent;
    else if (s == ":domain")
        return Domain;
    else if (s == ":parameters")
        return Parameters;
    else if (s == ":input")
        return Input;
    else if (s == ":output")
        return Output;
    else if (s == ":precondition")
        return PreCondition;
    else if (s == ":postcondition")
        return PostCondition;
    else if (s == ":effect")
        return Effect;
    else if (s == ":requirements")
        return Requirements;
    else if (s == ":types")
        return Types;
    else if (s == ":constants")
        return Constants;
    else if (s == ":predicates")
        return Predicates;
    else if (s == ":strips")
        return Strips;
    else if (s == ":typing")
        return Typing;
    else if (s == "object")
        return Object;
    else if (s == "number")
        return Number;
    else if (s == "string")
        return SString;
    else if (s == "and")
        return And;
    else if (s == "or")
        return Or;
    else if (s == "not")
        return Not;
    else if (s == "null")
        return Null;
    else if (s == "start")
        return START;
    else if (s == "-")
        return Dash;
    else if (s == "(")
        return OpenParen;
    else if (s == ")")
        return CloseParen;
    else if (s == ">")
        return GreaterThan;
    else if (s == "<")
        return LessThan;
    else if (s == "=")
        return Equal;
    else if (s == ">=")
        return GE;
    else if (s == "<=")
        return LE;
    else if (s == "!=")
        return NE;
    else if (s == "::")
        return VariableSeparator;
    else if (s == ":fluents")
        return Fluent;
    else if (s == ":functions")
        return Functions;
    else if (s == "assign")
        return Assign;
    else if (s == "increase")
        return Increase;
    else if (s == "decrease")
        return Decrease;
    else
        return -1;
}

bool Tokens::isSymbol(int c)
{
    return ( c == ':' ) || ( c == '(' ) || ( c == ')' ) || ( c == '<' ) || ( c == '>' ) || ( c == '=' ) || ( c == '!' ) || ( c == '?' ) || ( c == '-' );
};

bool Tokens::isComparator(int c)
{
    return ( c == GE ) || ( c == LE ) || ( c == Equal ) || ( c == GreaterThan ) || ( c == LessThan ) || ( c == NE );
};

bool Tokens::isWhitespace(int c)
{
    return isspace(c);
};

string Tokens::getVariableName(int token)
{
    if (variablesRev.find(token) != variablesRev.end())
        return variablesRev.at(token);
    else
    {
        if (!getName(token).empty())
            return getName(token);
        else
            return "token " + token;
    }
};

bool Tokens::isNumber(int c)
{
    string intString = getVariableName(c);
    try
    {
        long x = stol(intString);
    }
    catch (invalid_argument ia1)
    {
        try
        {
            double x = stod(intString);
        }
        catch (invalid_argument ia2)
        {
            try
            {
                float x = stof(intString);
            }
            catch (invalid_argument ia3)
            {
                return false;
            }
        }
        return true;
    }
    return true;
};

int Tokens::getToken(string s)
{
    Poco::toLowerInPlace(s);
    if (getTokenHelper(s) != -1)
        return getTokenHelper(s);
    int varToken = variables.at(s);
    if (varToken != 0)
        return varToken;
    return NA;
};

int Tokens::getNextChar()
{
    string tempString;
    int retChar;
    if(inputStream.gcount() != 0 && getInputString().empty())
    {
        try
        {
            retChar = inputStream.get();
            return retChar;
        }
        catch(ifstream::failure e)
        {
            cerr << "WARNING: Exception caught: " << e.what();
            return -1;
        }
    }
    else if(inputStream.gcount() == 0 && !getInputString().empty())
    {
        tempString = getInputString();
        if(currentPos < tempString.length())
        {
            return tempString.at(currentPos++);
        }
        else
            return -1;
    }
    else
    {
        cerr << "WARNING: PPDLParser -- inputString and inputStream cannot both be empty/not-empty";
        return 0;
    }
};

string Tokens::getNextWord()
{
    int c;
    string s = "";
    int token;

    // Sanity check (not sure what that is)
    if (inputStream.gcount() == 0 && getInputString().empty())
        return "";

    // Get lookahead for the first time
    if (lookahead == -2)
        lookahead = getNextChar();
    else if (lookahead == -1) // this is the end of the file
        return "";

    while (true)
    { // Get the next character
        c = lookahead;
        if (c == -1) // eof
            break;
        lookahead = -2;

        // Only getting one word, so stop once whitespace is hit
        if (isWhitespace(c) && s.length() == 0)
        {
            lookahead = getNextChar();
            continue;
        }
        if (isWhitespace(c))
            break;
        if (isSymbol(c))
        {
            //longest symbolic token is two characters
            if (s.length() == 0)
            {
                lookahead = getNextChar();
                string checkString = to_string(c) + to_string(lookahead);
                token = getToken(checkString);
                if(token != NA)
                {
                    s = checkString;
                    lookahead = getNextChar();
                    break; //breakout if the first non-whitespace is token
                }
                checkString = c;
                token = getToken(checkString);
                s = checkString;
                if(token != NA)
                    break; //breakout if the first non-whitespace is token
                else
                    continue;
            }
            else if (s.length() == 1)
            {
                // If second non-whitespace, check if two characters together make token. If not, treat as whitespace
                string checkString = s + to_string(c);
                token = getToken(checkString);
                if (token != NA)
                {
                    s = checkString;
                    break;
                }
                else
                {
                    lookahead = c;
                    break;
                }
            }
            else
            {
                // if the string is already 2 characters long, treat it as whitespace
                lookahead = c;
                break;
            }
        }
        s += to_string(c);
        lookahead = getNextChar();
    }

    return s;
};

int Tokens::getNextToken()
{
    string s = getNextWord();
    if (s.empty())
        nextToken = NA;
    else
        nextToken = tokenize(s);
    return nextToken;
};

int Tokens::tokenize(string s)
{
    Poco::toLowerInPlace(s);
    int token = getToken(s);
    if(token == NA)
    { // Is there even a point to this? It only checks one value...it is stupid but whatever
        token = nextVariable;
        variables.emplace(s, nextVariable++);
        variablesRev.emplace(token, s);
        return variables.at(s);
    }
    return token;
};