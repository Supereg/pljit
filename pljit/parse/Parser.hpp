//
// Created by Andreas Bauer on 28.07.22.
//

#ifndef PLJIT_PARSER_HPP
#define PLJIT_PARSER_HPP

#include "../code/SourceCode.hpp"
#include "../lex/Lexer.hpp"
#include "../util/Result.hpp"
#include "./ParseTree.hpp"
#include <optional>

//---------------------------------------------------------------------------
namespace pljit::parse {
//---------------------------------------------------------------------------
class Parser {
    lex::Lexer* lexer;

    public:
    explicit Parser(lex::Lexer& lexer);

    Result<FunctionDefinition> parse_program();

    Result<FunctionDefinition> parseFunctionDefinition();

    Result<ParameterDeclarations> parseParameterDeclarations();
    Result<VariableDeclarations> parseVariableDeclarations();
    Result<ConstantDeclarations> parseConstantDeclarations();

    Result<DeclaratorList> parseDeclaratorList();
    Result<InitDeclaratorList> parseInitDeclaratorList();
    Result<InitDeclarator> parseInitDeclarator();

    Result<CompoundStatement> parseCompoundStatement();
    Result<StatementList> parseStatementList();
    Result<Statement> parseStatement();

    Result<AssignmentExpression> parseAssignmentExpression();
    Result<AdditiveExpression> parseAdditiveExpression();
    Result<MultiplicativeExpression> parseMultiplicativeExpression();

    Result<UnaryExpression> parseUnaryExpression();
    Result<PrimaryExpression> parsePrimaryExpression();

    Result<Identifier> parseIdentifier();
    Result<Literal> parseLiteral();
    Result<GenericTerminal> parseGenericTerminal(
        lex::Token::Type expected_type,
        std::string_view expected_content,
        std::string_view potential_error_message
    );
};
//---------------------------------------------------------------------------
} // namespace pljit::parse
//---------------------------------------------------------------------------

#endif //PLJIT_PARSER_HPP
