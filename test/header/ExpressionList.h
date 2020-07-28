#ifndef EXPRESSIONLIST_H
#define EXPRESSIONLIST_H

#include "Expression.h"
#include <vector>

class ParserAgentDefinition;

class ExpressionList : public Tokens
{
private:
	ParserAgentDefinition* parserAgentDefinition;

public:
	std::vector<Expression> expressionList;

	ExpressionList();
	ExpressionList(const ExpressionList& dup); //Copy constructor

	int size() { return expressionList.size(); };
	bool empty() { return expressionList.empty(); };

	void set(ParserAgentDefinition agentDef, std::string field, std::string value);
	Expression get(int index);
	void add(Expression expr);
	void addExpression(Expression newExpr);
	void dump(std::unordered_map<int, std::string> dictionary);

	int getExpressionVariable(int var);
	int getExpressionBaseVariable(int baseVar);
	bool equalExpressionLists(ExpressionList list2);
	bool containsExpressionList(ExpressionList list2);
};

#endif