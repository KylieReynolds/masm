#include "..\header\ExpressionList.h"
#include "..\header\ParserAgentDefinition.h"
#include <poco/String.h>

using namespace std;

ExpressionList::ExpressionList() { expressionList; };
ExpressionList::ExpressionList(const ExpressionList& dup) { expressionList = dup.expressionList; };

void ExpressionList::set(ParserAgentDefinition agentDef, string field, string value)
{
	Expression expr;
	expr.setVariable(tokenize(field));
	expr.setBaseVariable(tokenize(agentDef.swapVariableDomain(field)));
	expr.setComparator(Equal);

	int valueType = agentDef.getType(expr.getVariable());
	if(valueType == SString)
		expr.setString(value);
	else if(valueType == Number)
		expr.setValue(stof(value));
	else if(valueType == Boolean)
		expr.setBoolean(Poco::toLower(value).compare("equal") == 0);
	else if(valueType == Object)
		expr.setToken(tokenize(value));

	addExpression(expr);
};

Expression ExpressionList::get(int index)
{
	if(expressionList.size() > index)
		return expressionList.at(index);
	else
		return {}; // return null
};

void ExpressionList::add(Expression expr)
{
	if (expressionList.size() != 0)
		expressionList.emplace_back(expr);
};

void ExpressionList::addExpression(Expression newExpr)
{
	vector<Expression>::iterator it;
	for (it = expressionList.begin(); it != expressionList.end(); it++)
	{
		int index = distance(expressionList.begin(), it);
		Expression oldExpr = *it;
		if (oldExpr.getBaseVariable() == newExpr.getBaseVariable())
		{
			if (oldExpr.dontCare())
			{
				oldExpr = newExpr;
				return;
			}
			else
			{
				if (oldExpr.getComparator() == GreaterThan && oldExpr.isNum() && newExpr.getComparator() == LessThan && newExpr.isNum() && oldExpr.getValue() < newExpr.getValue())
				{
					newExpr.setComparator(RangeGTLT);
					newExpr.setStartValue(oldExpr.getValue());
					newExpr.setEndValue(newExpr.getValue());
				}
				else if (oldExpr.getComparator() == GE && oldExpr.isNum() && newExpr.getComparator() == LE &&
					newExpr.isNum() && oldExpr.getValue() <= newExpr.getValue())
				{
					newExpr.setComparator(RangeGELE);
					newExpr.setStartValue(oldExpr.getValue());
					newExpr.setEndValue(newExpr.getValue());
				}
				else if (oldExpr.getComparator() == GreaterThan && oldExpr.isNum() && newExpr.getComparator() == LE &&
					newExpr.isNum() && oldExpr.getValue() < newExpr.getValue())
				{
					newExpr.setComparator(RangeGTLE);
					newExpr.setStartValue(oldExpr.getValue());
					newExpr.setEndValue(newExpr.getValue());
				}
				else if (oldExpr.getComparator() == GE && oldExpr.isNum() && newExpr.getComparator() == LessThan &&
					newExpr.isNum() && oldExpr.getValue() < newExpr.getValue())
				{
					newExpr.setComparator(RangeGELT);
					newExpr.setStartValue(oldExpr.getValue());
					newExpr.setEndValue(newExpr.getValue());
				}
				else if (oldExpr.getComparator() == LessThan && oldExpr.isNum() && newExpr.getComparator() == GreaterThan &&
					newExpr.isNum() && oldExpr.getValue() > newExpr.getValue())
				{
					newExpr.setComparator(RangeGTLT);
					newExpr.setStartValue(newExpr.getValue());
					newExpr.setEndValue(oldExpr.getValue());
				}
				else if (oldExpr.getComparator() == LE && oldExpr.isNum() && newExpr.getComparator() == GE &&
					newExpr.isNum() && oldExpr.getValue() >= newExpr.getValue())
				{
					newExpr.setComparator(RangeGELE);
					newExpr.setStartValue(newExpr.getValue());
					newExpr.setEndValue(oldExpr.getValue());
				}
				else if (oldExpr.getComparator() == LE && oldExpr.isNum() && newExpr.getComparator() == GreaterThan &&
					newExpr.isNum() && oldExpr.getValue() > newExpr.getValue())
				{
					newExpr.setComparator(RangeGTLT);
					newExpr.setStartValue(newExpr.getValue());
					newExpr.setEndValue(oldExpr.getValue());
				}
				else if (oldExpr.getComparator() == LessThan && oldExpr.isNum() && newExpr.getComparator() == GE &&
					newExpr.isNum() && oldExpr.getValue() > newExpr.getValue())
				{
					newExpr.setComparator(RangeGELT);
					newExpr.setStartValue(newExpr.getValue());
					newExpr.setEndValue(oldExpr.getValue());
				}
				else
				{
					cerr << "WARNING: Already defined a condition for existing expression";
					return;
				}
				oldExpr = newExpr;
				return;
			}
		}
	}
	expressionList.emplace_back(newExpr);
};

void ExpressionList::dump(unordered_map<int, string> dictionary)
{
	if (expressionList.empty())
		cout << "null/empty\n";
	else
	{
		vector<Expression>::iterator it;
		for(it = expressionList.begin(); it != expressionList.end(); it++)
		{
			Expression expr = *it;
			cout << expr.toString(dictionary) << '\n';
		}
	}
};

int ExpressionList::getExpressionVariable(int var)
{
	vector<Expression>::iterator it;
	for(it = expressionList.begin(); it != expressionList.end(); it++)
	{
		int index = distance(expressionList.begin(), it);
		Expression expr = *it;
		if (var == expr.getVariable())
			return index;
	}
	return -1;
};

int ExpressionList::getExpressionBaseVariable(int baseVar)
{
	vector<Expression>::iterator it;
	for(it = expressionList.begin(); it != expressionList.end(); it++)
	{
		int index = distance(expressionList.begin(), it);
		Expression expr = *it;
		if (baseVar == expr.getBaseVariable())
			return index;
	}
	return -1;
};

bool ExpressionList::equalExpressionLists(ExpressionList list2)
{
	if (expressionList.empty() || list2.empty() || (expressionList.size() != list2.size()))
		return false;

	vector<Expression>::iterator it;
	for(it = expressionList.begin(); it != expressionList.end(); it++)
	{
		Expression expr1 = *it;
		int index = list2.getExpressionVariable(expr1.getVariable());
		if (index == -1)
			return false;

		Expression expr2 = list2.get(index);

		if (expr1.getBaseVariable() != expr2.getBaseVariable() || expr1.getVariable() != expr2.getVariable() ||
			expr1.getComparator() != expr2.getComparator() || expr1.getValue() != expr2.getValue() ||
			expr1.getString() != expr2.getString() || expr1.getToken() != expr2.getToken() ||
			expr1.getStartValue() != expr2.getStartValue() || expr1.getEndValue() != expr2.getEndValue())
		{
			return false;
		}
	}
	return true;
};

bool ExpressionList::containsExpressionList(ExpressionList list2)
{
	if (expressionList.empty() || list2.empty())
		return false;

	vector<Expression>::iterator it;
	for(it = expressionList.begin(); it != expressionList.end(); it++)
	{
		Expression expr1 = *it;
		if (expr1.dontCare())
			continue;

		int pubIndex = list2.getExpressionBaseVariable(expr1.getBaseVariable());
		if (pubIndex == -1)
			return false;

		Expression expr2 = list2.get(pubIndex);
		if (expr2.dontCare() || Expression::matchExpressions(expr1, expr2) == occurAlways ||
			( Expression::matchExpressions(expr1, expr2) == occurSometimes && expr2.getComparator() == Equal ))
			continue;

		return false;
	}
	return true;
};