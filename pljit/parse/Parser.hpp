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
class Parser {
    lex::Lexer* lexer;

    public:
    // TODO make constructivle from SRCMNGMT?
    explicit Parser(lex::Lexer& lexer);

    Result<FunctionDefinition> parse_program();

    [[nodiscard]] std::optional<code::SourceCodeError> parseFunctionDefinition(FunctionDefinition& destination);

    [[nodiscard]] std::optional<code::SourceCodeError> parseParameterDeclarations(std::optional<ParameterDeclarations>& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseVariableDeclarations(std::optional<VariableDeclarations>& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseConstantDeclarations(std::optional<ConstantDeclarations>& destination);

    [[nodiscard]] std::optional<code::SourceCodeError> parseDeclaratorList(DeclaratorList& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseInitDeclaratorList(InitDeclaratorList& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseInitDeclarator(InitDeclarator& destination);

    [[nodiscard]] std::optional<code::SourceCodeError> parseCompoundStatement(CompoundStatement& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseStatementList(StatementList& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseStatement(Statement& destination);

    [[nodiscard]] std::optional<code::SourceCodeError> parseAssignmentExpression(AssignmentExpression& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseAdditiveExpression(AdditiveExpression& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseMultiplicativeExpression(MultiplicativeExpression& destination);

    [[nodiscard]] std::optional<code::SourceCodeError> parseUnaryExpression(UnaryExpression& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parsePrimaryExpression(PrimaryExpression& destination);

    [[nodiscard]] std::optional<code::SourceCodeError> parseIdentifier(Identifier& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseLiteral(Literal& destination);
    [[nodiscard]] std::optional<code::SourceCodeError> parseGenericTerminal(
        GenericTerminal& destination,
        lex::Token::Type expected_type,
        std::string_view expected_content,
        std::string_view potential_error_message
    );
};
//---------------------------------------------------------------------------
} // namespace pljit::parse
//---------------------------------------------------------------------------

#endif //PLJIT_PARSER_HPP
