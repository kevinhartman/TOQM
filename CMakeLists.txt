cmake_minimum_required(VERSION 3.11...3.22)

# Project name and a few useful settings. Other commands can pick up the results
project("libtoqm"
        VERSION 0.1.0
        DESCRIPTION
        "Chi Zhang, Ari B. Hayes, Longfei Qiu, Yuwei Jin, Yanhao Chen, and Eddy Z. Zhang. 2021. Time-Optimal Qubit \
        Mapping. In Proceedings of the 26th ACM International Conference on Architectural Support for Programming \
        Languages and Operating Systems (ASPLOS ’21), April 19–23, 2021, Virtual, USA. \
        ACM, New York, NY, USA, 14 pages."
        HOMEPAGE_URL "https://doi.org/10.1145/3445814.3446706"
        LANGUAGES CXX)

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here

    # Let's ensure -std=c++xx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Let's nicely support folders in IDEs
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Testing only available if this is the main app
    # Note this needs to be done in the main CMakeLists
    # since it calls enable_testing, which must be in the
    # main CMakeLists.
    # TODO: uncomment after adding testing
    #include(CTest)
endif()

# The compiled library code is here
add_subdirectory(libtoqm/libtoqm)

# The executable code is here
add_subdirectory(mapper)

# Testing only available if this is the main app
# Emergency override MODERN_CMAKE_BUILD_TESTING provided as well
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
        AND BUILD_TESTING)
    # TODO: uncomment after adding testing
    #add_subdirectory(tests)
endif()
