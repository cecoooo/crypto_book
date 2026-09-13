add_executable(order_book_tests
    test_main.cpp
    pair_utils_tests.cpp
    price_level_container_tests.cpp
    book_tests.cpp
    book_container_tests.cpp
    engine_tests.cpp
    boundary_tests.cpp
    invalid_input_tests.cpp
)

target_include_directories(order_book_tests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(order_book_tests PRIVATE cxx_std_17)

target_compile_definitions(order_book_tests
    PRIVATE
        TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

target_link_libraries(order_book_tests
    PRIVATE
        engine
        data_types
        utils
)

set_target_properties(order_book_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME order_book_tests COMMAND order_book_tests)
