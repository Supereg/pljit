#include <iostream>
#include <string>

#include "./code/SourceCodeManagement.hpp"
#include "./lex/Lexer.hpp"

//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------

// TODO remove main file!

void print(const pljit::lex::Token& token) {
    switch (token.getType()) {
        case pljit::lex::Token::Type::EMPTY:
            cout << "EMPTY" << endl;
            break;
        default:
            cout << "'" << token.reference().content() << "'" << endl;
    }
}

// TODO introduce additional target for executable!
// TODO -fno-rtti (no run time type information)
// TODO optimizations (e.g. with division by zero) -> ensure that the error is still there
// TODO introduce multiple namespaces for different components(?) + top level namespace!!
//  e.g. parsing in e.g. separate namespace
// TODO separate into library and executable (CLI/frontend)
// TODO include directories (e.g. #include "foo/header.h")
// TODO check variable ordering (bigger to smaller!)

int main() {
    std::string program = "PARAM width, height, depth;";

    pljit::code::SourceCodeManagement management{ std::move(program) };
    pljit::lex::Lexer lexer{management};

    while(true) {
        pljit::Result<pljit::lex::Token> result = lexer.consume_next();
        print(result.value());

        result = lexer.consume_next();
        print(result.value());

        auto error = pljit::code::SourceCodeError(pljit::code::ErrorType::ERROR, "Hello. This is invalid!", result.value().reference());
        error.printCompilerError();

        result = lexer.consume_next();
        print(result.value());

        result = lexer.consume_next();
        print(result.value());

        result = lexer.consume_next();
        print(result.value());

        result = lexer.consume_next();
        print(result.value());

        result = lexer.consume_next();
        print(result.value());

        result = lexer.consume_next();
        print(result.value());
        break;
    }
}
//---------------------------------------------------------------------------
