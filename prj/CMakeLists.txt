cmake_minimum_required(VERSION 3.16)

project(OrderBookProject
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(data_types)
add_subdirectory(engine)

add_executable(order_book_app
    main.cpp
)

target_link_libraries(order_book_app
    PRIVATE
        data_types
        engine
)

set_target_properties(order_book_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)