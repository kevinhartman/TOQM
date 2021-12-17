add_executable(toqm-console main.cpp)
set_target_properties(toqm-console PROPERTIES OUTPUT_NAME toqm)
target_compile_features(toqm-console PRIVATE cxx_std_11)

target_link_libraries(toqm-console PRIVATE toqm)
