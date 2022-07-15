#include <iostream>
#include <string>

#include "SourceCodeManagement.hpp"
#include "Lexer.hpp"
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------

void print(const pljit::Token& token) {
    switch (token.getType()) {
        case pljit::Token::TokenType::EMPTY:
            cout << "EMPTY" << endl;
            break;
        default:
            cout << "'" << token.reference().content() << "'" << endl;
    }
}

// TODO introduce multiple namespaces for different components(?) + top level namespace!!
//  e.g. parsing in e.g. separate namespace
// TODO separate into library and executable (CLI/frontend)
// TODO include directories (e.g. #include "foo/header.h")

int main() {
    std::string program = "PARAM width, height, depth;";

    pljit::SourceCodeManagement management{ std::move(program) };
    pljit::Lexer lexer{management};

    while(true) {
        pljit::Token token;
        lexer.next(token);
        print(token);

        token = {};
        lexer.next(token);
        print(token);

        token.reference().print_error(pljit::SourceCodeManagement::ErrorType::ERROR, "Hello. This is invalid!");

        token = {};
        lexer.next(token);
        print(token);

        break;
    }
}
//---------------------------------------------------------------------------
