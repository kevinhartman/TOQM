find_package(Catch2 REQUIRED)

include(CTest)
include(Catch)

add_executable(toqm-test
        TestLatency.cpp
        main.cpp
        util/ComparisonHelpers.cpp
        util/MapperBuilder.cpp
        util/PrinterHelpers.cpp)

target_include_directories(toqm-test PRIVATE ${CMAKE_SOURCE_DIR}/libtoqm)

target_link_libraries(toqm-test PUBLIC toqm Catch2::Catch2)

catch_discover_tests(toqm-test)