add_executable(empty main.cpp suite_a.cpp suite_b.cpp)
target_include_directories(empty PRIVATE ${CMAKE_SOURCE_DIR}/litetest)
target_link_libraries(empty PRIVATE litetest)