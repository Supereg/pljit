//
// Created by Andreas Bauer on 28.07.22.
//

#ifndef PLJIT_PARSER_HPP
#define PLJIT_PARSER_HPP

#include "SourceCodeManagement.hpp"
#include "Lexer.hpp"
#include "Result.hpp"
#include <memory>
#include <optional>
#include <vector>
#include <concepts> // TODO required?

namespace pljit {
class Parser;
}

// TODO how to organize namespaces??
namespace pljit::ParseTree { // TODO add ability to print to graph!
//---------------------------------------------------------------------------
class Symbol {
    public:
    Symbol() = default;
};

class GenericTerminal : public Symbol {
    SourceCodeReference reference;

    public:
    GenericTerminal();
    GenericTerminal(SourceCodeReference reference);
};

class Identifier: public Symbol {
    SourceCodeReference reference;
    public:
    Identifier();
    Identifier(SourceCodeReference reference);
};

class Literal: public Symbol {
    SourceCodeReference reference;
    long long literalValue; // 64-bit integer

    public:
    Literal();
    Literal(SourceCodeReference reference, long long literalValue);
};

class PrimaryExpression: public Symbol {
    friend class pljit::Parser;

    /// Describes the content type of the `symbols` property.
    enum class Type {
        NONE, // TODO NONE types?
        /// PrimaryExpression contains a single instance of `Identifier`
        IDENTIFIER,
        /// PrimaryExpression contains a single instance of `Literal`
        LITERAL,
        /// PrimaryExpression contains a `GenericTerminal` "(", `AdditiveExpression` and `GenericTerminal` ")"
        ADDITIVE_EXPRESSION,
    };

    Type type;
    std::vector<std::unique_ptr<Symbol>> symbols;

    public:
    PrimaryExpression();
};

class UnaryExpression: public Symbol {
    friend class pljit::Parser; // TODO can we avoid all the friend definitions in this file?

    std::optional<GenericTerminal> unaryOperator;
    PrimaryExpression primaryExpression;

    public:
    UnaryExpression();
};


class MultiplicativeExpression: public Symbol {
    friend class pljit::Parser;

    UnaryExpression expression;
    std::vector<std::tuple<GenericTerminal, MultiplicativeExpression>> optionalOperand;

    public:
    MultiplicativeExpression();
};

class AdditiveExpression: public Symbol {
    friend class pljit::Parser;

    MultiplicativeExpression expression;

    std::vector<std::tuple<GenericTerminal, AdditiveExpression>> optionalOperand;

    public:
    AdditiveExpression();
};

class AssignmentExpression: public Symbol {
    friend class pljit::Parser;

    Identifier identifier;
    GenericTerminal assignmentOperator; // `:=`
    AdditiveExpression additiveExpression;

    public:
    AssignmentExpression();
};

class Statement: public Symbol {
    friend class pljit::Parser;

    /// Describes the content type of the `symbols` property.
    enum class Type {
        NONE,
        /// Statement contains a single instance of `AssignmentExpression`.
        ASSIGNMENT,
        /// Statement contains a `GenericTerminal` "RETURN" and an `AdditiveExpression`.
        RETURN,
    };

    Type type;
    // TODO what the hell!
    std::vector<std::unique_ptr<Symbol>> symbols;

    public:
    Statement();
};

class StatementList: public Symbol {
    friend class pljit::Parser;

    Statement statement;
    std::vector<std::tuple<GenericTerminal, Statement>> additionalStatements;

    public:
    StatementList();
};

class CompoundStatement: public Symbol {
    friend class pljit::Parser;

    GenericTerminal beginKeyword;
    StatementList statementList;
    GenericTerminal endKeyword;

    public:
    CompoundStatement();
};

class InitDeclarator: public Symbol {
    friend class pljit::Parser;

    Identifier identifier;
    GenericTerminal assignmentOperator; // `=`
    Literal literal;

    public:
    InitDeclarator();
};

class InitDeclaratorList: public Symbol {
    friend class pljit::Parser;

    InitDeclarator initDeclarator;
    std::vector<std::tuple<GenericTerminal, InitDeclarator>> additionalInitDeclarators;

    public:
    InitDeclaratorList();
};

class DeclaratorList: public Symbol {
    friend class pljit::Parser;

    Identifier identifier;
    std::vector<std::tuple<GenericTerminal, Identifier>> additionalIdentifiers;

    public:
    DeclaratorList();
};

class ConstantDeclarations: public Symbol {
    friend class pljit::Parser;

    GenericTerminal constKeyword;
    InitDeclaratorList initDeclaratorList;
    GenericTerminal semicolon;

    public:
    ConstantDeclarations();
};

class VariableDeclarations: public Symbol {
    friend class pljit::Parser;

    GenericTerminal varKeyword;
    DeclaratorList declaratorList;
    GenericTerminal semicolon;

    public:
    VariableDeclarations();
};

class ParameterDeclarations: public Symbol {
    friend class pljit::Parser;

    GenericTerminal paramKeyword;
    DeclaratorList declaratorList;
    GenericTerminal semicolon;

    public:
    ParameterDeclarations();
};

class FunctionDefinition: public Symbol { // TODO public inheritance isn't what we want right?
    friend class pljit::Parser;

    std::optional<ParameterDeclarations> parameterDeclarations;
    std::optional<VariableDeclarations> variableDeclarations;
    std::optional<ConstantDeclarations> constantDeclarations;

    CompoundStatement compoundStatement;

    public:
    FunctionDefinition();
    // TODO GenericTerminal terminator; // TODO endOfprogramm Terminator!?!?
};
//---------------------------------------------------------------------------
} // namespace pljit::ParseTree
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
class Parser { // TODO "RecursiveDescentParser"
    // TODO one function for every non-terminal!
    Lexer* lexer;

    public:
    explicit Parser(Lexer& lexer);

    // TODO make this const sends the wrong signal, does it?
    Result<ParseTree::FunctionDefinition> parse() const;

    private:
    [[nodiscard]] std::optional<SourceCodeError> parseFunctionDefinition(ParseTree::FunctionDefinition& destination) const;

    [[nodiscard]] std::optional<SourceCodeError> parseParameterDeclarations(std::optional<ParseTree::ParameterDeclarations>& destination) const;
    [[nodiscard]] std::optional<SourceCodeError> parseVariableDeclarations(std::optional<ParseTree::VariableDeclarations>& destination) const;
    [[nodiscard]] std::optional<SourceCodeError> parseConstantDeclarations(std::optional<ParseTree::ConstantDeclarations>& destination) const;

    [[nodiscard]] std::optional<SourceCodeError> parseDeclaratorList(ParseTree::DeclaratorList& destination) const;
    [[nodiscard]] std::optional<SourceCodeError> parseInitDeclaratorList(ParseTree::InitDeclaratorList& destination) const;
    [[nodiscard]] std::optional<SourceCodeError> parseInitDeclarator(ParseTree::InitDeclarator& destination) const;

    [[nodiscard]] std::optional<SourceCodeError> parseCompoundStatement(ParseTree::CompoundStatement& destination) const;
    [[nodiscard]] std::optional<SourceCodeError> parseStatementList(ParseTree::StatementList& destination) const;
    [[nodiscard]] std::optional<SourceCodeError> parseStatement(ParseTree::Statement& destination) const;

    [[nodiscard]] std::optional<SourceCodeError> parseAssignmentExpression(ParseTree::AssignmentExpression& destination) const;
    [[nodiscard]] std::optional<SourceCodeError> parseAdditiveExpression(ParseTree::AdditiveExpression& destination) const;
    [[nodiscard]] std::optional<SourceCodeError> parseMultiplicativeExpression(ParseTree::MultiplicativeExpression& destination) const;

    [[nodiscard]] std::optional<SourceCodeError> parseUnaryExpression(ParseTree::UnaryExpression& destination) const;
    [[nodiscard]] std::optional<SourceCodeError> parsePrimaryExpression(ParseTree::PrimaryExpression& destination) const;

    [[nodiscard]] std::optional<SourceCodeError> parseIdentifier(ParseTree::Identifier& destination) const;
    [[nodiscard]] std::optional<SourceCodeError> parseLiteral(ParseTree::Literal& destination) const;
    [[nodiscard]] std::optional<SourceCodeError> parseGenericTerminal(
        ParseTree::GenericTerminal& destination,
        Token::TokenType expected_type,
        std::string_view expected_content,
        std::string_view potential_error_message
    ) const;
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_PARSER_HPP
