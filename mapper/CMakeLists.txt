add_executable(mapper main.cpp)
target_compile_features(mapper PRIVATE cxx_std_11)

target_link_libraries(mapper PRIVATE toqm)
