set(PLJIT_SOURCES
    # add your source files here
    main.cpp
    code/SourceCodeManagement.cpp
    lex/Lexer.cpp
    parse/Parser.cpp
    parse/ParseTreeDOTVisitor.cpp
    ast/AST.cpp
    ast/ASTBuilder.cpp
    SymbolTable.cpp
    ast/ASTDOTVisitor.cpp
    util/GenericDOTVisitor.cpp
    parse/ParseTree.cpp
    pljit.cpp
    EvaluationContext.cpp optimizations/DeadCodeElimination.cpp optimizations/ConstantPropagation.cpp optimizations/ConstantPropagation.hpp PljitFunction.cpp PljitFunction.hpp)

add_library(pljit_core ${PLJIT_SOURCES})
target_include_directories(pljit_core PUBLIC ${CMAKE_SOURCE_DIR})

add_clang_tidy_target(lint_pljit_core ${PLJIT_SOURCES})
add_dependencies(lint lint_pljit_core)

add_executable(pljit main.cpp)
target_link_libraries(pljit PUBLIC pljit_core)
