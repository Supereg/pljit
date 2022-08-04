set(PLJIT_SOURCES
    # add your source files here
    main.cpp
    SourceCodeManagement.cpp SourceCodeManagement.hpp Lexer.cpp Lexer.hpp Parser.cpp Parser.hpp Result.hpp ParseTreeVisitor.hpp ParseTreeDOTVisitor.cpp ParseTreeDOTVisitor.hpp AST.cpp AST.hpp ASTBuilder.cpp ASTBuilder.hpp SymbolTable.cpp SymbolTable.hpp ASTVisitor.hpp ASTDOTVisitor.cpp ASTDOTVisitor.hpp GenericDOTVisitor.cpp GenericDOTVisitor.hpp)

add_library(pljit_core ${PLJIT_SOURCES})
target_include_directories(pljit_core PUBLIC ${CMAKE_SOURCE_DIR})

add_clang_tidy_target(lint_pljit_core ${PLJIT_SOURCES})
add_dependencies(lint lint_pljit_core)

add_executable(pljit main.cpp)
target_link_libraries(pljit PUBLIC pljit_core)
