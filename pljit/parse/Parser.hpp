//
// Created by Andreas Bauer on 28.07.22.
//

#ifndef PLJIT_PARSER_HPP
#define PLJIT_PARSER_HPP

#include "../code/SourceCodeManagement.hpp"
#include "../lex/Lexer.hpp"
#include "../util/Result.hpp"
#include "./ParseTree.hpp"
#include <optional>

//---------------------------------------------------------------------------
namespace pljit::parse {
//---------------------------------------------------------------------------
class Parser { // TODO "RecursiveDescentParser"
    lex::Lexer* lexer;

    public:
    explicit Parser(lex::Lexer& lexer); // TODO manage lexer creation itself?

    Result<ParseTree::FunctionDefinition> parse_program();

    [[nodiscard]] std::optional<code::SourceCodeError> parseFunctionDefinition(ParseTree::FunctionDefinition& destination);

    [[nodiscard]] std::optional<code::SourceCodeError> parseParameterDeclarations(std::optional<ParseTree::ParameterDeclarations>& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseVariableDeclarations(std::optional<ParseTree::VariableDeclarations>& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseConstantDeclarations(std::optional<ParseTree::ConstantDeclarations>& destination);

    [[nodiscard]] std::optional<code::SourceCodeError> parseDeclaratorList(ParseTree::DeclaratorList& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseInitDeclaratorList(ParseTree::InitDeclaratorList& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseInitDeclarator(ParseTree::InitDeclarator& destination);

    [[nodiscard]] std::optional<code::SourceCodeError> parseCompoundStatement(ParseTree::CompoundStatement& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseStatementList(ParseTree::StatementList& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseStatement(ParseTree::Statement& destination);

    [[nodiscard]] std::optional<code::SourceCodeError> parseAssignmentExpression(ParseTree::AssignmentExpression& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseAdditiveExpression(ParseTree::AdditiveExpression& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseMultiplicativeExpression(ParseTree::MultiplicativeExpression& destination);

    [[nodiscard]] std::optional<code::SourceCodeError> parseUnaryExpression(ParseTree::UnaryExpression& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parsePrimaryExpression(ParseTree::PrimaryExpression& destination);

    [[nodiscard]] std::optional<code::SourceCodeError> parseIdentifier(ParseTree::Identifier& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseLiteral(ParseTree::Literal& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseGenericTerminal(
        ParseTree::GenericTerminal& destination,
        lex::Token::Type expected_type,
        std::string_view expected_content,
        std::string_view potential_error_message
    );
};
//---------------------------------------------------------------------------
} // namespace pljit::parse
//---------------------------------------------------------------------------

#endif //PLJIT_PARSER_HPP
