add_library(data_types STATIC
    src/book.cpp
    src/price_level_container.cpp
    src/book_container.cpp
)

target_include_directories(data_types PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_compile_features(data_types PUBLIC cxx_std_17)
target_link_libraries(data_types PUBLIC utils)