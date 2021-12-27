# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

set(HEADER_LIST
        "CostFunc/CXFrontier.hpp"
        "CostFunc/CXFull.hpp"
        "CostFunc/SimpleCost.hpp"
        "Expander/DefaultExpander.hpp"
        "Expander/GreedyTopK.hpp"
        "Expander/NoSwaps.hpp"
        "Filter/HashFilter.hpp"
        "Filter/HashFilter2.hpp"
        "Latency/Latency_1.hpp"
        "Latency/Latency_1_2_6.hpp"
        "Latency/Latency_1_3.hpp"
        "Latency/Table.hpp"
        "NodeMod/GreedyMapper.hpp"
        "Queue/DefaultQueue.hpp"
        "Queue/TrimSlowNodes.hpp"
        "CostFunc.hpp"
        "Environment.hpp"
        "Expander.hpp"
        "Filter.hpp"
        "GateNode.hpp"
        "Latency.hpp"
        "LinkedStack.hpp"
        "myParser.hpp"
        "Node.hpp"
        "NodeMod.hpp"
        "Queue.hpp"
        "ScheduledGate.hpp"
        "Toqm.hpp"
        )

# Make an automatic library - will be static or dynamic based on user setting
add_library(toqm
        toqm.cpp
        Node.cpp
        myParser.cpp
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
