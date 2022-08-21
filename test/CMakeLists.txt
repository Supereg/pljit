set(TEST_SOURCES
    Tester.cpp
    SourceCodeManagementTests.cpp
    LexerTests.cpp
    ParserTests.cpp
    ASTTests.cpp
    PljitTests.cpp
    ASTOptimizationTests.cpp
    utils/ast_utils.cpp
    utils/CaptureCOut.cpp)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(tester ${TEST_SOURCES})
target_link_libraries(tester PUBLIC
    pljit_core
    GTest::GTest
    Threads::Threads)
