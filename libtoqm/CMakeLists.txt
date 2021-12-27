# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

set(HEADER_LIST
        "${TOQM_SOURCE_DIR}/libtoqm/CostFunc/CXFrontier.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/CostFunc/CXFull.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/CostFunc/SimpleCost.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Expander/DefaultExpander.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Expander/GreedyTopK.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Expander/NoSwaps.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Filter/HashFilter.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Filter/HashFilter2.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Latency/Latency_1.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Latency/Latency_1_2_6.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Latency/Latency_1_3.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Latency/Table.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/NodeMod/GreedyMapper.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Queue/DefaultQueue.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Queue/TrimSlowNodes.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/CostFunc.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Environment.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Expander.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Filter.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/GateNode.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Latency.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/LinkedStack.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/myParser.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Node.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/NodeMod.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Queue.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/ScheduledGate.hpp"
        "${TOQM_SOURCE_DIR}/libtoqm/Toqm.hpp"
        )

# Make an automatic library - will be static or dynamic based on user setting
add_library(toqm
        toqm.cpp
        Node.cpp
        myParser.cpp
        ${HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(toqm PUBLIC ${TOQM_SOURCE_DIR})

# All users of this library will need at least C++11
target_compile_features(toqm PUBLIC cxx_std_11)

# IDEs should put the headers in a nice place
#source_group(
#        TREE "${PROJECT_SOURCE_DIR}/include"
#        PREFIX "Header Files"
#        FILES ${HEADER_LIST})
