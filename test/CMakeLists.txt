# CMakeList.txt
cmake_minimum_required (VERSION 3.15)

project ("mas_middleware")

#Included directories
include_directories(src)


# Add source to this project's executable.
add_executable (mas_middleware "mas_middleware.cpp" "mas_middleware.h"
	"src/Act3Formatter.cpp"
	"src/AgentBase.cpp"
	"src/AgentFactory.cpp"
	"src/AgentUtil.cpp"
	"src/AgentSpawner.cpp"
	"src/ArgumentParser.cpp"
	"src/Expression.cpp"
	"src/ExpressionList.cpp"
	"src/HitmanAgent.cpp"
	"src/Mas.cpp"
	"src/NodeCommunicationLayer.cpp"
	"src/ParserAgentDefinition.cpp"
	"src/PDDLParser.cpp"
	"src/PDDLParserAgent.cpp"
	"src/PDDLParserDomain.cpp"
	"src/SpyAgent.cpp"
	"src/Tokens.cpp"
	"src/UtilitiesVector.cpp"
	"src/ZyreCommunicationLayer.cpp"
)

#Add libraries
add_library(headers INTERFACE)
target_include_directories(headers INTERFACE header/)

add_library(poco INTERFACE)
target_include_directories(poco INTERFACE include/)

add_library(protos INTERFACE)
target_include_directories(protos INTERFACE include/)

add_library(thread INTERFACE) #intel thread building blocks
target_include_directories(thread INTERFACE include/)

add_library(zeromq INTERFACE)
target_include_directories(zeromq INTERFACE include/)

# Link libraries to executable
target_link_libraries(mas_middleware headers poco protos thread zeromq)
target_link_libraries(headers INTERFACE protos)
target_link_libraries(headers INTERFACE thread)
target_link_libraries(headers INTERFACE zeromq)