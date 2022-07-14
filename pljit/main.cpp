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
            cout << "'" << token.content().content() << "'" << endl;
    }
}

int main() {
    std::string program = "PARAM width, height, depth;";

    pljit::SourceCodeManagement management{ program };
    pljit::Lexer lexer{management};

    while(true) {
        pljit::Token token;
        lexer.next(token);
        print(token);

        token = {};
        lexer.next(token);
        print(token);

        token = {};
        lexer.next(token);
        print(token);

        break;
    }
}
//---------------------------------------------------------------------------
