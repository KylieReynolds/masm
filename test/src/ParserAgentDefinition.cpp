#include "..\header\ExpressionList.h"
#include "..\header\ParserAgentDefinition.h"
#include <sstream>
#include <poco/String.h>

using namespace std;

ParserAgentDefinition::ParserAgentDefinition(string aName, int anId, Tokens aTokenParser, PDDLParserDomain aDomainParser)
{
	name = aName;
	id = anId;
	tokenParser = aTokenParser;
	domainParser = aDomainParser;
	definition = getInputString();
};

ParserAgentDefinition::ParserAgentDefinition(const ParserAgentDefinition& pad)
{
	name = pad.name;
	id = pad.id;
	tokenParser = pad.tokenParser;
	domainParser = pad.domainParser;
	definition = pad.definition;
};

string ParserAgentDefinition::toString()
{
	ostringstream oss;
	oss << "\n\nAgent " << id << " - " << name;

	oss << "\n\tDomains:\t";
	for (int dom : domains)
		oss << getVariableName(dom) << '\t';

	oss << "\n\tParameters:\n";
	for (auto const& p : parameters)
		oss << "\t\t" << getVariableName(p.first) << ": " << getVariableName(p.second) << '\n';

	oss << "\tInputs:\n";
	for (auto const& i : inputs)
		oss << "\t\t" << getVariableName(i.first) << ": " << getVariableName(i.second) << '\n';

	oss << "\tOutputs:\n";
	for (auto const& o : outputs)
		oss << "\t\t" << getVariableName(o.first) << ": " << getVariableName(o.second) << '\n';

	oss << "\tPreconditions:\t";
	for (int preId : preconditions)
		oss << getVariableName(preId) << '\t';

	oss << "\n\tPostconditions:\t";
	for (int postId : postconditions)
		oss << getVariableName(postId) << '\t';

	oss << "\n\n";
	return oss.str();
};

// Swap the variable name with its domain name -- "fci::height" into "image::height" (assuming fci is in the image domain)
string ParserAgentDefinition::swapVariableDomain(string fullname)
{
	if (fullname.find("::") <= 0)
		return fullname; // no values to swap

	string varName = Poco::toLower(fullname.substr(0, fullname.find("::")));
	int varId = tokenize(varName);

	int domId = inputs.find(varId) == inputs.end() ? -1 : inputs.at(varId);
	if (domId == -1)
		domId = outputs.find(varId) == outputs.end() ? -1 : outputs.at(varId);
	if (domId == -1)
		domId = parameters.find(varId) == parameters.end() ? -1 : parameters.at(varId);

	string newName = (domId == -1 ? "" : getVariableName(domId)) + "::" + Poco::toLower(fullname.substr(fullname.find("::") + 2));
	return newName;
};

// Returns an expression list that contains each predicate for an agent's domains and compare to NA token value
ExpressionList ParserAgentDefinition::getPredicateSet(map<int, int> io)
{
	// Build an expression list of all of the variables for the input/output -- Each variable starts with a value of tokenNA
	ExpressionList curPredicates;
	map<int, int>::iterator ioIt;
	for(ioIt = io.begin(); ioIt != io.end(); ioIt++)
	{
		int var$ = ioIt->first;
		int domId = ioIt->second;

		// Check if domains/predicates for domId are empty
		DomainDefinition domDef = domainParser.getDomain(domId);
		map<int, PredicateDefinition> dPred = domDef.getPredicates();
		if (dPred.empty())
			return curPredicates;

		map<int, PredicateDefinition>::iterator predIt;
		for(predIt = dPred.begin(); predIt != dPred.end(); predIt++) // For each predicate in domain
		{
			PredicateDefinition predDef = predIt->second;
			if (predDef.size() > 1)	// if more than 1 variable for predicate
			{
				map<int, int>::iterator varIt;
				for(varIt = predDef.variables.begin(); varIt != predDef.variables.end(); varIt++) // All variables in predicate
				{
					string varName = getVariableName(domId) + "::" + getVariableName(predIt->first) + "::" + getVariableName(varIt->first);
					string domName = swapVariableDomain(varName);
					Expression newExpr;
					newExpr.setVariable(tokenize(varName));
					newExpr.setBaseVariable(tokenize(domName));
					curPredicates.add(newExpr);
				}
			}
			else
			{
				int baseVar = predIt->first;
				string varName = getVariableName(var$) + "::" + getVariableName(baseVar);
				string domName = swapVariableDomain(varName);
				Expression newExpr;
				newExpr.setVariable(tokenize(varName));
				newExpr.setBaseVariable(tokenize(domName));
				curPredicates.add(newExpr);
			}
		}
	}
	return curPredicates;
};

// Get the type of the token variable
int ParserAgentDefinition::getType(int token)
{
	string aname = getVariableName(token);
	if (isPredefined(token) || aname.size() == 0 || aname.find("::") == -1)
		return NA;

	string domInstance = aname.substr(0, aname.find("::")); // domain is before ::
	int instanceId = getToken(domInstance);
	if (instanceId == NA)
		return NA;

	int domId;
	if (inputs.find(instanceId) != inputs.end())
		domId = inputs.at(instanceId);
	else if (outputs.find(instanceId) != outputs.end())
		domId = outputs.at(instanceId);
	else if (parameters.find(instanceId) != parameters.end())
		domId = parameters.at(instanceId);
	else
		return NA;

	int type = NA;
	string varName = aname.substr(aname.find("::") + 2);	// predicate is after ::
	int apredicate = getToken(varName);
	if (varName.find("::") <= 0)
	{
		type = domainParser.getPredicateType(domId, apredicate);
		if (type == NA && domainParser.isFunction(apredicate))
			return Number;
	}
	else
	{
		string predicateName = varName.substr(0, varName.find("::"));
		if (domainParser.isFunction(apredicate))
			return Number;
		varName = aname.substr(varName.find("::") + 2);
		int avariable = getToken(varName);
		type = domainParser.getPredicateType(domId, apredicate, avariable);
	}

	if (type == SString || type == Number || type == Boolean || type == Object)
		return type;

	return NA;
};

Expression ParserAgentDefinition::getExpression(int field1, string field1_name, string field1_domain,
	int field2, string field2_name, string field2_domain, int comparison)
{
	Expression returnExp;
	int valueType;

	// One of the fields (and ONLY one) needs to be a variable (format is variable::domain)
	if (( field1_name.find("::") > 0 && field2_name.find("::") > 0 ) ||
		( field1_name.find("::") <= 0 && field2_name.find("::") <= 0 ))
		return {}; // return null

	else if (field1_name.find("::") > 0)
	{
		returnExp.setVariable(field1);	// variable name with the instance in front
		returnExp.setBaseVariable(getToken(field1_domain));	// variable name with the domain in front
		valueType = getType(field1);
		if (valueType == SString)
			returnExp.setString(field2_name);
		else if (valueType == Boolean)
			returnExp.setBoolean(Poco::toLower(field2_name).compare("true") == 0);
		else if (valueType == Number)
			returnExp.setValue(stof(field2_name));
		else if (valueType == Object)
			returnExp.setToken(field2);
		else
			return {}; // return null
	}
	else
	{
		returnExp.setVariable(field2);
		returnExp.setBaseVariable(getToken(field2_domain));
		valueType = getType(field2);
		if (valueType == SString)
			returnExp.setString(field1_name);
		else if (valueType == Boolean)
			returnExp.setBoolean(Poco::toLower(field1_name).compare("true") == 0);
		else if (valueType == Number)
			returnExp.setValue(stof(field1_name));
		else if (valueType == Object)
			returnExp.setToken(field1);
		else
			return {}; // return null
	}

	returnExp.setComparator(comparison);
	return returnExp;
};

// Build a list of conditions that match pre- or post- conditions
ExpressionList ParserAgentDefinition::getExpressionList(vector<int> fieldList, int domainVariable)
{
	if (domainParser.getPredicateDomainCount(domainVariable) != 1)
		return {}; // return null

	int domVarPosition = domainParser.getPredicateDomainPosition(domainVariable);
	int domVarField = fieldList.at(domVarPosition);

	ExpressionList returnList;
	PredicateDefinition predicates = domainParser.getPredicates(domainVariable);

	// Construct field name from domain and predicate
	Expression returnExp;
	returnExp.setComparator(Equal);
	string fieldName = getVariableName(domVarField) + "::" + getVariableName(domainVariable);
	returnExp.setVariable(tokenize(fieldName));
	returnExp.setBaseVariable(tokenize(swapVariableDomain(fieldName)));

	if (predicates.size() == 0)
		return {}; // return null
	else if (predicates.size() == 1 || predicates.size() == 2)
	{
		if (predicates.size() == 1)
			returnExp.setBoolean(true);
		else
		{
			int field = ( domVarPosition == 0 ? fieldList.at(1) : fieldList.at(0) );
			int valueType = getType(returnExp.getVariable());
			string fieldName = getVariableName(field);
			if (valueType == SString)
				returnExp.setString(fieldName);
			else if (valueType == Boolean)
				returnExp.setBoolean(Poco::toLower(fieldName).compare("true") == 0);
			else if (valueType == Number)
				returnExp.setValue(stof(fieldName));
			else if (valueType == Object)
				returnExp.setToken(field);
			else
				return {}; // return null
		}
		returnList.add(returnExp);
		return returnList;
	}
	else
	{
		int count = 0;
		map<int, int>::iterator x;
		for(x = predicates.variables.begin(); x != predicates.variables.end(); x++)
		{
			int variable = x->first; // get key
			int type$ = x->second;   // get value
			if (variable == domVarField)
			{
				count++;
				continue;
			}

			int field = fieldList.at(count);
			int valueType = getType(returnExp.getVariable());
			if (valueType == SString)
				returnExp.setString(fieldName);
			else if (valueType == Boolean)
				returnExp.setBoolean(Poco::toLower(fieldName).compare("true") == 0);
			else if (valueType == Number)
				returnExp.setValue(stof(fieldName));
			else if (valueType == Object)
				returnExp.setToken(field);
			else
				return {}; // return null
			returnList.add(returnExp);
			count++;
		}
	}
	return returnList;
};

// Build set of sets of expressions where each internal set is a sufficient condition for pre/post to be satisfied
vector<ExpressionList> ParserAgentDefinition::buildList(deque<int> conditions, map<int, int> io)
{
	deque<TwoDimExpressionList> curDeque;
	TwoDimExpressionList curList;

	deque<int>::iterator it;
	for(it = conditions.begin(); it != conditions.end(); it++)
	{
		int curField = *it;
		curList.conditionList.clear(); // make null

		if (domainParser.isPredicate(curField) || domainParser.isFunction(curField))
		{
			curList.boolValue = true;
			curDeque.push_back(curList);
			continue;
		}
		else if (curField == True)
		{
			curList.boolValue = true;
			curDeque.push_back(curList);
			continue;
		}
		else if (curField == False)
		{
			curList.boolValue = false;
			curDeque.push_back(curList);
			continue;
		}
		else if (curField != And && curField != Or)
		{
			if (it == conditions.end()) // Should be impossible
			{
				cerr << "WARNING: PDDLParser -- agent compatibility -- malformed condition -- missing argument";
				return {}; // return null;
			}

			vector<int> fieldList;
			int nextToken = NA;
			while (it != conditions.end())
			{
				nextToken = *it;
				if (nextToken == Assign)
					nextToken = Equal;

				if (domainParser.isFunction(nextToken))
				{
					string tempName = getVariableName(curField) + "::" + getVariableName(nextToken);
					curField = tokenize(tempName);
					continue;
				}

				if (domainParser.isPredicate(nextToken) || isComparator(nextToken))
					break; // All the arguments have been obtained

				fieldList.push_back(curField);
				curField = nextToken;
				it++; // Advance iterator
			}

			fieldList.push_back(curField);
			if (isComparator(nextToken))
			{
				if (fieldList.size() != 2)
				{
					cerr << "WARNING: PDDLParser -- agent compatibility -- too many operands for comparator";
					return {}; // return null
				}

				int field1 = fieldList.at(0);
				string field1_name = getVariableName(field1);
				string field1_domain = swapVariableDomain(field1_name);
				tokenize(field1_domain);

				int field2 = fieldList.at(1);
				string field2_name = getVariableName(field2);
				string field2_domain = swapVariableDomain(field2_name);
				tokenize(field2_domain);

				Expression newExpr = getExpression(field1, field1_name, field1_domain, field2, field2_name, field2_domain, nextToken);
				ExpressionList curPreds = getPredicateSet(io);	// List of predicates with tokenNA in each
				curPreds.addExpression(newExpr);
				curList.conditionList.push_back(curPreds);
				curList.boolValue = false;
				curDeque.push_back(curList);
				continue;
			}
			else
			{
				if (domainParser.getPredicateDomainCount(nextToken) == 0)
				{
					curList.boolValue = true;
					curDeque.push_back(curList);
					continue;
				}
				else if(domainParser.getPredicateDomainCount(nextToken) > 1)
				{
					cerr << "WARNING: PDDLParser -- agent compatibility -- malformed condition -- two domain variables in condition";
					return {};	// return null
				}
			}
		}
		else
		{
			if (curDeque.size() < 2)
			{
				cerr << "WARNING: PDDLParser -- agent compatibility -- AND/OR encountered without two arguments";
				return {}; // return null
			}
			TwoDimExpressionList expr1 = curDeque.front();
			curDeque.pop_front();
			TwoDimExpressionList expr2 = curDeque.front();
			curDeque.pop_front();

			if (curField == And)
			{
				if (expr1.conditionList.size() == 0)
				{
					if (expr1.boolValue)
					{
						curDeque.push_back(expr2);
						continue;
					}
					else
					{
						curList.boolValue = false;
						curDeque.push_back(curList);
						continue;
					}
				}
				else
				{
					if (expr2.conditionList.size() == 0)
					{
						if (expr2.boolValue)
						{
							curDeque.push_back(expr1);
							continue;
						}
						else
						{
							curList.boolValue = false;
							curDeque.push_back(curList);
							continue;
						}
					}
					else
					{
						for(int i = 0; i < expr1.conditionList.size(); i++)
						{
							for(int j = 0; j < expr2.conditionList.size(); j++)
							{
								for(int exprCount = 0; exprCount < expr2.conditionList.at(j).size(); exprCount++)
								{
									Expression expr = expr2.conditionList.at(j).get(exprCount);
									if(!expr.dontCare())
										expr1.conditionList.at(i).addExpression(expr);
								}
							}
						}
						curDeque.push_back(expr1);
						continue;
					}
				}
			}
			else if (curField == Or)
			{
				if (expr1.conditionList.size() == 0)
				{
					if (expr1.boolValue)
					{
						curList.boolValue = true;
						curDeque.push_back(curList);
						continue;
					}
					else
					{
						curDeque.push_back(expr2);
						continue;
					}
				}
				else
				{
					if (expr2.conditionList.size() == 0)
					{
						if (expr2.boolValue)
						{
							curList.boolValue = true;
							curDeque.push_back(curList);
							continue;
						}
						else
						{
							curDeque.push_back(expr1);
							continue;
						}
					}
					else
					{
						for (int i = 0; i < expr2.conditionList.size(); i++)
							expr1.conditionList.push_back(expr2.conditionList.at(i));
						curDeque.push_back(expr1);
						continue;
					}
				}
			}
		}
	}

	if (curDeque.size() == 1)
		return curDeque.front().conditionList;
	else
		return {}; // return null
};

vector<ExpressionList> ParserAgentDefinition::getPreconditions()
{
	vector<ExpressionList> retList = preconditionsMap;
	if (retList.empty())
	{
		if (inputs.empty())
			retList = buildList(preconditions, parameters);
		else
			retList = buildList(preconditions, inputs);
		preconditionsMap = retList;
	}
	return retList;
};

// check publishers postconditions to subscribers preconditions
AgentCompatibility ParserAgentDefinition::checkAgentCompatibility(ParserAgentDefinition subscriber)
{
	AgentCompatibility retCompat;

	// Deconstruct preconditions into sets of expression sets and postconditions into a set or preconditions
	if (subscriber.preconditions.empty() || postconditions.empty())
	{
		retCompat.setCompatibilityType(AgentCompatibilities::none);
		return retCompat;
	}

	vector<ExpressionList> subscriberSet = subscriber.buildList(subscriber.preconditions,
		subscriber.inputs.empty() ? subscriber.parameters : subscriber.inputs);
	vector<ExpressionList> publisherSet = buildList(postconditions, outputs.empty() ? parameters : outputs);
	if (subscriberSet.empty() || publisherSet.empty())
	{
		retCompat.setCompatibilityType(AgentCompatibilities::none);
		return retCompat;
	}

	ExpressionList partialSubConds;
	ExpressionList partialPubConds;

	vector<ExpressionList>::iterator itSubSet;
	for(itSubSet = subscriberSet.begin(); itSubSet != subscriberSet.end(); itSubSet++)
	{
		ExpressionList subConditions = *itSubSet;
		vector<ExpressionList>::iterator itPubSet;
		for(itPubSet = publisherSet.begin(); itPubSet != publisherSet.end(); itPubSet++)
		{
			ExpressionList pubConditions = *itPubSet;
			bool failed = false;
			bool partial = false;
			bool sometimes = false;
			for (auto const x : subConditions.expressionList)
			{
				Expression subExpression = x;
				if (subExpression.dontCare())
					continue;
				int pubIndex = pubConditions.getExpressionBaseVariable(subExpression.getBaseVariable());
				if (pubIndex == -1)
				{
					failed = true;
					break;
				}

				Expression pubExpression = pubConditions.get(pubIndex);
				if (pubExpression.dontCare())
					partial = true;

				if (Expression::matchExpressions(pubExpression, subExpression) == occurAlways)
					continue;
				else if (Expression::matchExpressions(pubExpression, subExpression) == occurSometimes)
				{
					sometimes = true;
					continue;
				}
				else
				{
					failed = true;
					break;
				}
			}

			if (!failed)
			{
				if (!partial)
				{
					retCompat.addCondition(pubConditions);
					retCompat.setCompatibilityType(sometimes ? AgentCompatibilities::sometimes : AgentCompatibilities::full);
				}

				partialSubConds = subConditions;
				partialPubConds = pubConditions;
			}
		}
	}

	if (!retCompat.getConditions().empty())
		return retCompat;

	if (!partialSubConds.expressionList.empty())
	{
		retCompat.addCondition(partialPubConds);
		retCompat.setCompatibilityType(AgentCompatibilities::partial);
		return retCompat;
	}

	retCompat.setCompatibilityType(static_cast<int>( AgentCompatibilities::none ));
	return retCompat;
};