set(TEST_SOURCES
    # add your source files here
    Tester.cpp
    SourceCodeManagementTests.cpp LexerTests.cpp utils/CaptureCOut.hpp)

add_executable(tester ${TEST_SOURCES})
target_link_libraries(tester PUBLIC
    pljit_core
    GTest::GTest)
