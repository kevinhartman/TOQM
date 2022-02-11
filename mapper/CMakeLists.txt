add_executable(mapper
        CouplingMap.hpp
        MapperUtils.h
        QasmObject.hpp
        CouplingMap.cpp
        MapperUtils.cpp
        QasmObject.cpp
        main.cpp)
target_compile_features(mapper PRIVATE cxx_std_11)

target_link_libraries(mapper PRIVATE toqm)
