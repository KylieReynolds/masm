#include "..\header\Expression.h"

using namespace std;

Expression::Expression()
{
    variable = NA;
    comparison = Equal;
    valueToken = NA;
    isNumber = false;
    isString = false;
    isBoolean = false;
};

void Expression::setValue(float val)
{
    valueNumber = val;
    isNumber = true;
    isString = false;
    isBoolean = false;
};
void Expression::setStartValue(float val)
{
    startValue = val;
    isNumber = true;
    isString = false;
    isBoolean = false;
};
void Expression::setEndValue(float val)
{
    startValue = val;
    isNumber = true;
    isString = false;
    isBoolean = false;
};

void Expression::setString(string val)
{
    valueString = val;
    isNumber = false;
    isString = true;
    isBoolean = false;
};

void Expression::setToken(int val)
{
    valueToken = val;
    isNumber = false;
    isString = false;
    isBoolean = false;
};

void Expression::setBoolean(bool val)
{
    valueBoolean = val;
    isNumber = false;
    isString = false;
    isBoolean = true;
};

bool Expression::dontCare()
{
    if (this == nullptr)
        return false;
    return ( comparison == Equal && !isNumber && !isString && !isBoolean && valueToken == NA );
};

string Expression::getVariableName(int token, unordered_map<int, string> dictionary)
{
    if (!dictionary.empty() && !( dictionary.at(token).length() > 0 ))
        return dictionary.at(token);
    if (Tokens::getName(token).length() > 0)
        return Tokens::getName(token);
    return "token " + token;
};

string Expression::toString(unordered_map<int, string> dictionary)
{
    char strBuf[ 255 ]; // Make a c-style string with max size 255 so that string can be formatted
    int formatString;

    if (comparison == RangeGTLT || comparison == RangeGELE || comparison == RangeGTLE || comparison == RangeGELT)
    {
        formatString = sprintf_s(strBuf, "%s %s %f, %f", getVariableName(baseVariable, dictionary).c_str(),
            getVariableName(comparison, dictionary).c_str(), startValue, endValue);
        return strBuf;
    }
    else
    {
        string type = (isNumber ? "number" : isString ? "string" : isBoolean ? "boolean" : "object");
        string value = to_string(isNumber ? valueNumber : isString ? stof(valueString) : isBoolean ? valueBoolean : stof(getVariableName(valueToken, dictionary)));
        formatString = sprintf_s(strBuf, "%s (%i type = %s)    %s    %s",
            getVariableName(baseVariable, dictionary).c_str(), baseVariable, type.c_str(),
            getVariableName(comparison, dictionary).c_str(), value.c_str());
        return strBuf;
    }
};

int Expression::matchExpressions(Expression pub, Expression sub)
{
    if((pub.baseVariable != sub.baseVariable) || (pub.isNumber != sub.isNumber) || (sub.isString != pub.isString) || (sub.isBoolean != pub.isBoolean))
        return occurNever;

    if(pub.isString)
    {
        if(pub.comparison == Equal)
            return (pub.valueString == sub.valueString ? occurAlways : occurNever);
        else
            return occurAlways; // there is a comment here that is ??????? so even the original coders don't know why this is here
    }
    else if (pub.isBoolean)
    {
        if (pub.comparison == Equal)
            return ( pub.valueBoolean == sub.valueBoolean ? occurAlways : occurNever );
        else
            return occurNever; // same ??????? comment here
    }
    else if (!pub.isNumber)
    {
        if (pub.comparison == Equal)
            return ( pub.valueToken == sub.valueToken ? occurAlways : occurNever );
        else
            return occurAlways; // same ??????? comment here
    }

    if (pub.comparison == Equal)
    {
        switch (sub.comparison)
        {
        case Equal:
            return ( sub.valueNumber == pub.valueNumber ? occurAlways : occurNever );
        case LessThan:
            return ( sub.valueNumber > pub.valueNumber ? occurAlways : occurNever );
        case LE:
            return ( sub.valueNumber >= pub.valueNumber ? occurAlways : occurNever );
        case GreaterThan:
            return ( sub.valueNumber < pub.valueNumber ? occurAlways : occurNever );
        case GE:
            return ( sub.valueNumber <= pub.valueNumber ? occurAlways : occurNever );
        case NE:
            return ( sub.valueNumber != pub.valueNumber ? occurAlways : occurNever );
        case RangeGTLT:
            return ( sub.startValue < pub.valueNumber && sub.endValue > pub.valueNumber ? occurAlways : occurNever );
        case RangeGELE:
            return ( sub.startValue <= pub.valueNumber && sub.endValue >= pub.valueNumber ? occurAlways : occurNever );
        case RangeGTLE:
            return ( sub.startValue < pub.valueNumber && sub.endValue >= pub.valueNumber ? occurAlways : occurNever );
        case RangeGELT:
            return ( sub.startValue <= pub.valueNumber && sub.endValue > pub.valueNumber ? occurAlways : occurNever );
        default:
            return occurNever;
        }
    }
    else if (pub.comparison == LessThan)
    {
        switch (sub.comparison)
        {
        case Equal:
            return ( sub.valueNumber < pub.valueNumber ? occurSometimes : occurNever );
        case LessThan:
            return ( sub.valueNumber >= pub.valueNumber ? occurAlways : occurSometimes );
        case LE:
            return ( sub.valueNumber >= pub.valueNumber ? occurAlways : occurSometimes );
        case GreaterThan:
            return ( sub.valueNumber < pub.valueNumber ? occurSometimes : occurNever );
        case GE:
            return ( sub.valueNumber < pub.valueNumber ? occurSometimes : occurNever );
        case NE:
            return ( sub.valueNumber >= pub.valueNumber ? occurAlways : occurSometimes );
        case RangeGTLT:
            return ( sub.startValue < pub.valueNumber ? occurSometimes : occurNever );
        case RangeGELE:
            return ( sub.startValue < pub.valueNumber ? occurSometimes : occurNever );
        case RangeGTLE:
            return ( sub.startValue < pub.valueNumber ? occurSometimes : occurNever );
        case RangeGELT:
            return ( sub.startValue < pub.valueNumber ? occurSometimes : occurNever );
        default:
            return occurNever;
        }
    }
    else if (pub.comparison == LE)
    {
        switch (sub.comparison)
        {
        case Equal:
            return ( sub.valueNumber <= pub.valueNumber ? occurSometimes : occurNever );
        case LessThan:
            return ( sub.valueNumber > pub.valueNumber ? occurAlways : occurSometimes );
        case LE:
            return ( sub.valueNumber >= pub.valueNumber ? occurAlways : occurSometimes );
        case GreaterThan:
            return ( sub.valueNumber < pub.valueNumber ? occurSometimes : occurNever );
        case GE:
            return ( sub.valueNumber < pub.valueNumber ? occurSometimes : occurNever );
        case NE:
            return ( sub.valueNumber > pub.valueNumber ? occurAlways : occurSometimes );
        case RangeGTLT:
            return ( sub.valueNumber < pub.valueNumber ? occurSometimes : occurNever );
        case RangeGELE:
            return ( sub.valueNumber <= pub.valueNumber ? occurSometimes : occurNever );
        case RangeGTLE:
            return ( sub.valueNumber < pub.valueNumber ? occurSometimes : occurNever );
        case RangeGELT:
            return ( sub.valueNumber <= pub.valueNumber ? occurSometimes : occurNever );
        default:
            return occurNever;
        }
    }
    else if (pub.comparison == GreaterThan)
    {
        switch (sub.comparison)
        {
        case Equal:
            return ( sub.valueNumber > pub.valueNumber ? occurSometimes : occurNever );
        case LessThan:
            return ( sub.valueNumber > pub.valueNumber ? occurSometimes : occurNever );
        case LE:
            return ( sub.valueNumber > pub.valueNumber ? occurSometimes : occurNever );
        case GreaterThan:
            return ( sub.valueNumber <= pub.valueNumber ? occurAlways : occurSometimes );
        case GE:
            return ( sub.valueNumber <= pub.valueNumber ? occurAlways : occurSometimes );
        case NE:
            return ( sub.valueNumber <= pub.valueNumber ? occurAlways : occurSometimes );
        case RangeGTLT:
            return ( sub.endValue > sub.valueNumber ? occurSometimes : occurNever );
        case RangeGELE:
            return ( sub.endValue > sub.valueNumber ? occurSometimes : occurNever );
        case RangeGTLE:
            return ( sub.endValue > sub.valueNumber ? occurSometimes : occurNever );
        case RangeGELT:
            return ( sub.endValue > sub.valueNumber ? occurSometimes : occurNever );
        default:
            return occurNever;
        }
    }
    else if (pub.comparison == GE)
    {
        switch (sub.comparison)
        {
        case Equal:
            return ( sub.valueNumber >= pub.valueNumber ? occurSometimes : occurNever );
        case LessThan:
            return ( sub.valueNumber > pub.valueNumber ? occurSometimes : occurNever );
        case LE:
            return ( sub.valueNumber > pub.valueNumber ? occurSometimes : occurNever );
        case GreaterThan:
            return ( sub.valueNumber < pub.valueNumber ? occurAlways : occurSometimes );
        case GE:
            return ( sub.valueNumber <= pub.valueNumber ? occurAlways : occurSometimes );
        case NE:
            return ( sub.valueNumber < pub.valueNumber ? occurAlways : occurSometimes );
        case RangeGTLT:
            return ( sub.endValue > pub.valueNumber ? occurSometimes : occurNever );
        case RangeGELE:
            return ( sub.endValue >= pub.valueNumber ? occurSometimes : occurNever );
        case RangeGTLE:
            return ( sub.endValue > pub.valueNumber ? occurSometimes : occurNever );
        case RangeGELT:
            return ( sub.endValue >= pub.valueNumber ? occurSometimes : occurNever );
        default:
            return occurNever;
        }
    }
    else if (pub.comparison == NE)
    {
        switch (sub.comparison)
        {
        case Equal:
            return ( sub.valueNumber == pub.valueNumber ? occurNever : occurAlways );
        case LessThan:
            return occurSometimes;
        case LE:
            return occurSometimes;
        case GreaterThan:
            return occurSometimes;
        case GE:
            return occurSometimes;
        case NE:
            return ( sub.valueNumber == pub.valueNumber ? occurAlways : occurSometimes );
        case RangeGTLT:
            return ( sub.startValue > pub.valueNumber || sub.endValue < pub.valueNumber ? occurAlways : occurSometimes );
        case RangeGELE:
            return ( sub.startValue == pub.valueNumber && sub.endValue == pub.valueNumber ? occurNever : sub.startValue > pub.valueNumber || sub.endValue < pub.valueNumber ? occurAlways : occurSometimes );
        case RangeGTLE:
            return ( sub.startValue > pub.valueNumber || sub.endValue < pub.valueNumber ? occurAlways : occurSometimes );
        case RangeGELT:
            return ( sub.startValue > pub.valueNumber || sub.endValue < pub.valueNumber ? occurAlways : occurSometimes );
        default:
            return occurNever;
        }
    }
    else if (pub.comparison == RangeGTLT)
    {
        switch (sub.comparison)
        {
        case Equal:
            return ( sub.valueNumber > pub.startValue&& sub.valueNumber < pub.endValue ? occurSometimes : occurNever );
        case LessThan:
            return ( sub.valueNumber > pub.startValue ? occurNever : occurSometimes );
        case LE:
            return ( sub.valueNumber > pub.startValue ? occurNever : occurSometimes );
        case GreaterThan:
            return ( sub.valueNumber < pub.endValue ? occurNever : occurSometimes );
        case GE:
            return ( sub.valueNumber < pub.endValue ? occurNever : occurSometimes );
        case RangeGTLT:
            return ( sub.startValue > pub.startValue&& sub.endValue < pub.endValue ? occurAlways : sub.startValue > pub.endValue || sub.endValue < pub.startValue ? occurNever : occurSometimes );
        case RangeGELE:
            return ( sub.startValue > pub.startValue&& sub.endValue < pub.endValue ? occurAlways : sub.startValue > pub.endValue || sub.endValue < pub.startValue ? occurNever : occurSometimes );
        case RangeGTLE:
            return ( sub.startValue > pub.startValue&& sub.endValue < pub.endValue ? occurAlways : sub.startValue > pub.endValue || sub.endValue < pub.startValue ? occurNever : occurSometimes );
        case RangeGELT:
            return ( sub.startValue > pub.startValue&& sub.endValue < pub.endValue ? occurAlways : sub.startValue > pub.endValue || sub.endValue < pub.startValue ? occurNever : occurSometimes );
        case NE:
            return ( sub.valueNumber < pub.startValue || sub.valueNumber > pub.endValue ? occurAlways : occurSometimes );
        default:
            return occurNever;
        }
    }
    else if (pub.comparison == RangeGELE)
    {
        switch (sub.comparison)
        {
        case Equal:
            return ( sub.valueNumber >= pub.startValue && sub.valueNumber <= pub.endValue ? occurSometimes : occurNever );
        case LessThan:
            return ( sub.valueNumber > pub.startValue ? occurNever : occurSometimes );
        case LE:
            return ( sub.valueNumber > pub.startValue ? occurNever : occurSometimes );
        case GreaterThan:
            return ( sub.valueNumber < pub.endValue ? occurNever : occurSometimes );
        case GE:
            return ( sub.valueNumber < pub.endValue ? occurNever : occurSometimes );
        case RangeGTLT:
            return ( sub.startValue > pub.startValue&& sub.endValue < pub.endValue ? occurAlways : sub.startValue > pub.endValue || sub.endValue < pub.startValue ? occurNever : occurSometimes );
        case RangeGELE:
            return ( sub.startValue > pub.startValue&& sub.endValue < pub.endValue ? occurAlways : sub.startValue > pub.endValue || sub.endValue < pub.startValue ? occurNever : occurSometimes );
        case RangeGTLE:
            return ( sub.startValue > pub.startValue&& sub.endValue < pub.endValue ? occurAlways : sub.startValue > pub.endValue || sub.endValue < pub.startValue ? occurNever : occurSometimes );
        case RangeGELT:
            return ( sub.startValue > pub.startValue&& sub.endValue < pub.endValue ? occurAlways : sub.startValue > pub.endValue || sub.endValue < pub.startValue ? occurNever : occurSometimes );
        case NE:
            return ( sub.valueNumber < pub.startValue || sub.valueNumber > pub.endValue ? occurAlways : occurSometimes );
        default:
            return occurNever;
        }
    }
    else if (pub.comparison == RangeGTLE)
    {
        switch (sub.comparison)
        {
        case Equal:
            return ( sub.valueNumber > pub.startValue&& sub.valueNumber < pub.endValue ? occurSometimes : occurNever );
        case LessThan:
            return ( sub.valueNumber > pub.startValue ? occurNever : occurSometimes );
        case LE:
            return ( sub.valueNumber > pub.startValue ? occurNever : occurSometimes );
        case GreaterThan:
            return ( sub.valueNumber < pub.endValue ? occurNever : occurSometimes );
        case GE:
            return ( sub.valueNumber < pub.endValue ? occurNever : occurSometimes );
        case RangeGTLT:
            return ( sub.startValue > pub.startValue&& sub.endValue < pub.endValue ? occurAlways : sub.startValue > pub.endValue || sub.endValue < pub.startValue ? occurNever : occurSometimes );
        case RangeGELE:
            return ( sub.startValue > pub.startValue&& sub.endValue < pub.endValue ? occurAlways : sub.startValue > pub.endValue || sub.endValue < pub.startValue ? occurNever : occurSometimes );
        case RangeGTLE:
            return ( sub.startValue > pub.startValue&& sub.endValue < pub.endValue ? occurAlways : sub.startValue > pub.endValue || sub.endValue < pub.startValue ? occurNever : occurSometimes );
        case RangeGELT:
            return ( sub.startValue > pub.startValue&& sub.endValue < pub.endValue ? occurAlways : sub.startValue > pub.endValue || sub.endValue < pub.startValue ? occurNever : occurSometimes );
        case NE:
            return ( sub.valueNumber < pub.startValue || sub.valueNumber > pub.endValue ? occurAlways : occurSometimes );
        default:
            return occurNever;
        }
    }
    else if (pub.comparison == RangeGELT)
    {
        switch (sub.comparison)
        {
        case Equal:
            return ( sub.valueNumber > pub.startValue&& sub.valueNumber < pub.endValue ? occurSometimes : occurNever );
        case LessThan:
            return ( sub.valueNumber > pub.startValue ? occurNever : occurSometimes );
        case LE:
            return ( sub.valueNumber > pub.startValue ? occurNever : occurSometimes );
        case GreaterThan:
            return ( sub.valueNumber < pub.endValue ? occurNever : occurSometimes );
        case GE:
            return ( sub.valueNumber < pub.endValue ? occurNever : occurSometimes );
        case RangeGTLT:
            return ( sub.startValue > pub.startValue&& sub.endValue < pub.endValue ? occurAlways : sub.startValue > pub.endValue || sub.endValue < pub.startValue ? occurNever : occurSometimes );
        case RangeGELE:
            return ( sub.startValue > pub.startValue&& sub.endValue < pub.endValue ? occurAlways : sub.startValue > pub.endValue || sub.endValue < pub.startValue ? occurNever : occurSometimes );
        case RangeGTLE:
            return ( sub.startValue > pub.startValue&& sub.endValue < pub.endValue ? occurAlways : sub.startValue > pub.endValue || sub.endValue < pub.startValue ? occurNever : occurSometimes );
        case RangeGELT:
            return ( sub.startValue > pub.startValue&& sub.endValue < pub.endValue ? occurAlways : sub.startValue > pub.endValue || sub.endValue < pub.startValue ? occurNever : occurSometimes );
        case NE:
            return ( sub.valueNumber < pub.startValue || sub.valueNumber > pub.endValue ? occurAlways : occurSometimes );
        default:
            return occurNever;
        }
    }

    return occurSometimes;
};