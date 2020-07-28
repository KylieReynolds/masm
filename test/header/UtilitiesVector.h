#ifndef UTILITIESVECTOR_H
#define UTILITIESVECTOR_H

#include "PDDLParser.h"
#include "PDDLParserDomain.h"
#include <protos/Communication.pb.h>

using namespace mas_middleware::protos;
namespace gp = google::protobuf;

class UtilitiesVector
{
public:
	/****		Never used and have nonexistant protobuf declarations		****/
	// static void setPredicate(WrapperMessage builder, std::string predicate, gp::Value predicateValue);
	// static bool hasPredicate(WrapperMessage wrapper, std::string predicate);
	// static gp::Value getPredicate(WrapperMessage wrapper, std::string predicate);

	std::list<std::string> getDatatypeList(PDDLParser parser, std::string domainDefDatatype);
	static gp::Value stringToData(std::string input, gp::Value::KindCase datatype);
	static gp::Value stringtoData(std::string input);
	static bool isDataEqual(gp::Value x, gp::Value y);
	static double dataToDouble(gp::Value datum);
	static std::string dataToString(gp::Value datum, char separator);
	static std::string dataToString(gp::Value datum);
	static std::string dataToString(std::list<gp::Value> data, char separator);
	static std::string dataToString(std::list<gp::Value> data);
	static std::string dataToStringWithType(gp::Value datum, char separator);
	static std::string dataToStringWithType(gp::Value datum);
	static std::string dataToStringWithType(std::list<gp::Value> data, char separator);
	static std::string dataToStringWithType(std::list<gp::Value> data);
	static gp::Value::KindCase stringToDatatypeValue(std::string datatype);
};

#endif