# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

set(HEADER_LIST
        "CommonTypes.hpp"
        "CostFunc.hpp"
        "CostFunc/CXFrontier.hpp"
        "CostFunc/CXFull.hpp"
        "CostFunc/SimpleCost.hpp"
        "CouplingMap.hpp"
        "Environment.hpp"
        "Expander.hpp"
        "Expander/DefaultExpander.hpp"
        "Expander/GreedyTopK.hpp"
        "Expander/NoSwaps.hpp"
        "Filter.hpp"
        "Filter/HashFilter.hpp"
        "Filter/HashFilter2.hpp"
        "GateNode.hpp"
        "Latency.hpp"
        "Latency/Latency_1.hpp"
        "Latency/Latency_1_2_6.hpp"
        "Latency/Latency_1_3.hpp"
        "Latency/Table.hpp"
        "LinkedStack.hpp"
        "Node.hpp"
        "NodeMod.hpp"
        "NodeMod/GreedyMapper.hpp"
        "QasmObject.hpp"
        "Queue.hpp"
        "Queue/DefaultQueue.hpp"
        "Queue/TrimSlowNodes.hpp"
        "ScheduledGate.hpp"
        "ToqmMapper.hpp"
        "ToqmResult.hpp")

# Make an automatic library - will be static or dynamic based on user setting
add_library(toqm
        CouplingMap.cpp
        Node.cpp
        QasmObject.cpp
        ToqmMapper.cpp
        ${HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(toqm PUBLIC ../)

# All users of this library will need at least C++11
target_compile_features(toqm PUBLIC cxx_std_11)

# IDEs should put the headers in a nice place
#source_group(
#        TREE "${PROJECT_SOURCE_DIR}/include"
#        PREFIX "Header Files"
#        FILES ${HEADER_LIST})
