add_library(engine STATIC
    src/engine.cpp
    src/update_reader.cpp
    src/snapshot_reader.cpp
    src/pair_utils.cpp
)

target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(engine PUBLIC cxx_std_17)
target_link_libraries(engine PUBLIC data_types)