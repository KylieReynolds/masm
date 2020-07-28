#ifndef AGENTCOMPATIBILITY_H
#define AGENTCOMPATIBILITY_H

#include "ExpressionList.h"
#include <vector>

enum AgentCompatibilities
{
    none = 1000,
    full,
    partial,
    sometimes
};

class AgentCompatibility
{
private:
    std::vector<ExpressionList> conditions;
    int compatibilityType;

public:
    AgentCompatibility() { compatibilityType = AgentCompatibilities::none; };
    std::vector<ExpressionList> getConditions() { return conditions; };
    int getCompatibilityType() { return compatibilityType; };
    void setCompatibilityType(int compatibility) { compatibilityType = compatibility; };

    void addCondition(ExpressionList exprList)
    {
        if (exprList.expressionList.size() != 0)
            conditions.emplace_back(exprList);
    }
    void clearConditions() { conditions.clear(); };
};

#endif