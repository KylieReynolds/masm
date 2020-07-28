// Class of static methods to simplify handling of vectors in the Communication protobuf
#include "../header/UtilitiesVector.h"
#include <poco/String.h>

using namespace std;

list<string> UtilitiesVector::getDatatypeList(PDDLParser parser, string domainDefDatatype)
{
	list<string> retList;
	PDDLParserDomain parserDomain = parser.getParserDomain();
	set<int>::iterator it;
	for(it = parserDomain.getDomainIds().begin(); it != parserDomain.getDomainIds().end(); it++)
	{
		int domainId = *it;
		map<string, string>::iterator itConst;
		for(itConst = parserDomain.getConstants(domainId).begin(); itConst != parserDomain.getConstants(domainId).end(); itConst++)
		{
			string sType = Poco::toLower(itConst->second);
			if(sType.compare(domainDefDatatype) == 0)
				retList.emplace_back(itConst->first);
		}
	}
	return retList;
};

gp::Value UtilitiesVector::stringToData(string input, gp::Value::KindCase datatype)
{
	gp::Value datum;
	Poco::trimInPlace(input);
	switch(datatype)
	{
	case datum.kNumberValue:
		datum.set_number_value(stod(input));
		break;
	
	case datum.kBoolValue:
		bool valueBool;
		if(Poco::toLower(input).compare("true") == 0)
			valueBool = true;
		else
			valueBool = false;
		datum.set_bool_value(valueBool);
		break;

	case datum.kStringValue:
	default:
		datum.set_string_value(input);
		break;
	}
	return datum;
};

gp::Value UtilitiesVector::stringtoData(string input)
{
	if(input.find(":") < 0)
		return {};	// return null
	int datatype = stoi(input.substr(0, input.find(":")));
	string value = input.substr(input.find(":") + 1);
	if(datatype == gp::Value::KindCase::kNumberValue)
		return stringToData(value, gp::Value::KindCase::kNumberValue);
	else if(datatype == gp::Value::KindCase::kBoolValue)
		return stringToData(value, gp::Value::KindCase::kBoolValue);
	else if(datatype == gp::Value::KindCase::kStringValue)
		return stringToData(value, gp::Value::KindCase::kStringValue);
	else
		return {};	// return null
};

bool UtilitiesVector::isDataEqual(gp::Value x, gp::Value y)
{
	if(x.has_list_value() || y.has_list_value())
	{
		if(x.has_list_value() != y.has_list_value())
			return false;
		else if(x.list_value().values_size() != y.list_value().values_size())
			return false;
		for(int i = 0; i < x.list_value().values_size(); i++)
		{
			if(!isDataEqual(x.list_value().values(i), y.list_value().values(i)))
				return false;
		}
		return true;
	}
	else if(x.kind_case() != y.kind_case())
		return false;

	switch(x.kind_case())
	{
	case gp::Value::KindCase::kNumberValue:
		return x.number_value() == y.number_value();
	case gp::Value::KindCase::kBoolValue:
		return x.bool_value() == y.bool_value();
	case gp::Value::KindCase::kStringValue:
		return x.string_value() == y.string_value();
	case gp::Value::KindCase::kNullValue:
	case gp::Value::KindCase::kStructValue:
	default:
		return true;
	}
};

double UtilitiesVector::dataToDouble(gp::Value datum)
{
	if(datum.kind_case() == gp::Value::KindCase::kStringValue)
		return stod(datum.string_value());
	return datum.number_value();
};

string UtilitiesVector::dataToString(gp::Value datum, char separator)
{
	if(datum.has_list_value())
	{
		list<gp::Value> x;
		for(int i = 0; i < datum.list_value().values_size(); i++)
			x.emplace_back(datum.list_value().values(i));
		return dataToString(x, separator);
	}
	else if(datum.kind_case() == gp::Value::KindCase::kNumberValue)
		return to_string(datum.number_value());
	else if(datum.kind_case() == gp::Value::KindCase::kBoolValue)
	{
		if(datum.bool_value() == true)
			return "true";
		else
			return "false";
	}
	else if(datum.kind_case() == gp::Value::KindCase::kStringValue)
		return datum.string_value();
	else
		return "";	// return null
};

string UtilitiesVector::dataToString(gp::Value datum)
{
	return dataToString(datum, ',');
};

string UtilitiesVector::dataToString(list<gp::Value> data, char separator)
{
	string str = to_string(separator);
	list<gp::Value>::iterator it;
	for(it = data.begin(); it != data.end(); it++)
	{
		gp::Value datum = *it;
		str += dataToString(datum);
	}
	return str;
};

string UtilitiesVector::dataToString(list<gp::Value> data)
{
	return dataToString(data, ',');
};

string UtilitiesVector::dataToStringWithType(gp::Value datum, char separator)
{
	if(datum.has_list_value())
	{
		list<gp::Value> x;
		for(int i = 0; i < datum.list_value().values_size(); i++)
			x.emplace_back(datum.list_value().values(i));
		return dataToString(x, separator);
	}
	else if(datum.kind_case() == gp::Value::KindCase::kNumberValue)
		return (gp::Value::KindCase::kNumberValue) + ":" + to_string(datum.number_value());
	else if(datum.kind_case() == gp::Value::KindCase::kBoolValue)
	{
		if(datum.bool_value() == true)
			return gp::Value::KindCase::kBoolValue + ":true";
		else
			return gp::Value::KindCase::kBoolValue + ":false";
	}
	else if(datum.kind_case() == gp::Value::KindCase::kStringValue)
		return gp::Value::KindCase::kStringValue + ":" + datum.string_value();
	else
		return "";	// return null
};

string UtilitiesVector::dataToStringWithType(gp::Value datum)
{
	return dataToStringWithType(datum, ',');
};

string UtilitiesVector::dataToStringWithType(list<gp::Value> data, char separator)
{
	string str = to_string(separator);
	list<gp::Value>::iterator it;
	for(it = data.begin(); it != data.end(); it++)
	{
		gp::Value datum = *it;
		str += dataToStringWithType(datum);
	}
	return str;
};

string UtilitiesVector::dataToStringWithType(list<gp::Value> data)
{
	return dataToStringWithType(data, ',');
};

gp::Value::KindCase UtilitiesVector::stringToDatatypeValue(string datatype)
{
	gp::Value::KindCase datatypeValue = gp::Value::KindCase::kNullValue;
	Poco::toLowerInPlace(datatype);
	if(datatype.compare("string") == 0)
		datatypeValue = gp::Value::KindCase::kStringValue;
	else if(datatype.compare("boolean") == 0)
		datatypeValue = gp::Value::KindCase::kBoolValue;
	else
		datatypeValue = gp::Value::KindCase::kNumberValue;
	return datatypeValue;
};