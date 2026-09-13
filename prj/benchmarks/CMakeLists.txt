add_executable(order_book_benchmark
    src/benchmark_main.cpp
    src/benchmark_memory.cpp
)

target_include_directories(order_book_benchmark
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(order_book_benchmark PRIVATE cxx_std_17)

target_link_libraries(order_book_benchmark
    PRIVATE
        engine
        data_types
        utils
)

if(WIN32)
    target_link_libraries(order_book_benchmark PRIVATE psapi)
endif()

set_target_properties(order_book_benchmark PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)