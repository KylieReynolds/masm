#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "Tokens.h"

class Expression
{
private:
    int variable{}, baseVariable{}, valueToken{}, comparison{};
    float valueNumber{}, startValue{}, endValue{};
    std::string valueString;
    bool valueBoolean{}, isNumber{}, isString{}, isBoolean{};

public:
    Expression();

    void setVariable(int var) { variable = var; };
    void setBaseVariable(int baseVar) { baseVariable = baseVar; };
    void setComparator(int compare) { comparison = compare; };
    void setValue(float val);
    void setStartValue(float val);
    void setEndValue(float val);
    void setString(std::string val);
    void setToken(int val);
    void setBoolean(bool val);

    int getVariable() { return variable; };
    int getBaseVariable() { return baseVariable; };
    int getComparator() { return comparison; };
    float getValue() { return valueNumber; };
    float getStartValue() { return startValue; };
    float getEndValue() { return endValue; };
    std::string getString() { return valueString; };
    bool getBoolean() { return valueBoolean; };
    int getToken() { return valueToken; };
    
    bool isNum() { return isNumber; };
    bool isStr() { return isString; };
    bool dontCare();

private:
    std::string getVariableName(int token, std::unordered_map<int, std::string> dictionary);

public:
    std::string toString(std::unordered_map<int, std::string> dictionary);
    static int matchExpressions(Expression pub, Expression sub);
};

#endif