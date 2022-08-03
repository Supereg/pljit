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
#include <gtest/gtest_prod.h>

namespace pljit {
class Parser;

// TODO how to organize namespaces??
namespace ParseTree { // TODO add ability to print to graph (=PrintVisitor?)!
//---------------------------------------------------------------------------
class ParseTreeVisitor;
class AdditiveExpression;
//---------------------------------------------------------------------------
class Symbol {
    friend class pljit::Parser;

    protected:
    SourceCodeReference src_reference; // TODO getter checks for nullptr!!!
    public:
    Symbol();
    Symbol(SourceCodeReference src_reference);

    virtual ~Symbol() = default;

    const SourceCodeReference& reference() const;

    virtual void accept(ParseTreeVisitor& visitor) const = 0;
};

class GenericTerminal: public Symbol {
    public:
    GenericTerminal();
    explicit GenericTerminal(SourceCodeReference src_reference);

    std::string_view value() const;
    void accept(ParseTreeVisitor& visitor) const override;
};

class Identifier: public Symbol {
    public:
    Identifier();

    std::string_view value() const;
    void accept(ParseTreeVisitor& visitor) const override;
};

class Literal: public Symbol {
    friend class pljit::Parser;
    long long literalValue; // 64-bit integer

    public:
    Literal();
    Literal(SourceCodeReference src_reference, long long literalValue);

    long long value() const;
    std::string_view string_value() const;

    void accept(ParseTreeVisitor& visitor) const override;
};

class PrimaryExpression: public Symbol {
    friend class pljit::Parser;

    public:
    /// Describes the content type of the `symbols` property.
    enum class Type {
        NONE,
        /// PrimaryExpression contains a single instance of `Identifier`
        IDENTIFIER,
        /// PrimaryExpression contains a single instance of `Literal`
        LITERAL,
        /// PrimaryExpression contains a `GenericTerminal` "(", `AdditiveExpression` and `GenericTerminal` ")"
        ADDITIVE_EXPRESSION,
    };

    private:
    Type type;
    std::vector<std::unique_ptr<Symbol>> symbols;

    public:
    PrimaryExpression();

    Type getType() const;

    const Identifier& asIdentifier() const;
    const Literal& asLiteral() const;
    std::tuple<const GenericTerminal&, const AdditiveExpression&, const GenericTerminal&> asBracketedExpression() const;

    void accept(ParseTreeVisitor& visitor) const override;
};

class UnaryExpression: public Symbol {
    friend class pljit::Parser; // TODO can we avoid all the friend definitions in this file?

    std::optional<GenericTerminal> unaryOperator;
    PrimaryExpression primaryExpression;

    public:
    UnaryExpression();

    const std::optional<GenericTerminal>& getUnaryOperator() const;
    const PrimaryExpression& getPrimaryExpression() const;

    void accept(ParseTreeVisitor& visitor) const override;
};


class MultiplicativeExpression: public Symbol {
    friend class pljit::Parser;

    UnaryExpression expression;
    std::vector<std::tuple<GenericTerminal, MultiplicativeExpression>> optionalOperand;

    public:
    MultiplicativeExpression();

    const UnaryExpression& getExpression() const;
    std::optional<std::tuple<const GenericTerminal&, const MultiplicativeExpression&>> getOperand() const;

    void accept(ParseTreeVisitor& visitor) const override;
};

class AdditiveExpression: public Symbol {
    friend class pljit::Parser;

    MultiplicativeExpression expression;

    std::vector<std::tuple<GenericTerminal, AdditiveExpression>> optionalOperand;

    public:
    AdditiveExpression();

    const MultiplicativeExpression& getExpression() const;
    std::optional<std::tuple<const GenericTerminal&, const AdditiveExpression&>> getOperand() const;

    void accept(ParseTreeVisitor& visitor) const override;
};

class AssignmentExpression: public Symbol {
    friend class pljit::Parser;

    Identifier identifier;
    GenericTerminal assignmentOperator;
    AdditiveExpression additiveExpression;

    public:
    AssignmentExpression();

    const Identifier& getIdentifier() const;
    const GenericTerminal& getAssignmentOperator() const;
    const AdditiveExpression& getAdditiveExpression() const;

    void accept(ParseTreeVisitor& visitor) const override;
};

class Statement: public Symbol {
    friend class pljit::Parser;

    public:
    /// Describes the content type of the `symbols` property.
    enum class Type {
        NONE,
        /// Statement contains a single instance of `AssignmentExpression`.
        ASSIGNMENT,
        /// Statement contains a `GenericTerminal` "RETURN" and an `AdditiveExpression`.
        RETURN,
    };

    private:
    Type type;
    std::vector<std::unique_ptr<Symbol>> symbols;

    public:
    Statement();

    /// Deleted copy constructor
    Statement(const Statement& other) = delete;
    /// Move constructor
    Statement(Statement&& other) = default;

    /// Deleted copy assignment
    Statement& operator=(const Statement& other) = delete;
    /// Move assignment
    Statement& operator=(Statement&& other) = default;

    Type getType() const;

    const AssignmentExpression& asAssignmentExpression() const;
    std::tuple<const GenericTerminal&, const AdditiveExpression&> asReturnExpression() const;

    void accept(ParseTreeVisitor& visitor) const override;
};

class StatementList: public Symbol {
    friend class pljit::Parser;

    Statement statement;
    std::vector<std::tuple<GenericTerminal, Statement>> additionalStatements;

    public:
    StatementList();

    const Statement& getStatement() const;
    const std::vector<std::tuple<GenericTerminal, Statement>>& getAdditionalStatements() const;

    void accept(ParseTreeVisitor& visitor) const override;
};

class CompoundStatement: public Symbol {
    friend class pljit::Parser;

    GenericTerminal beginKeyword;
    StatementList statementList;
    GenericTerminal endKeyword;

    public:
    CompoundStatement();

    const GenericTerminal& getBeginKeyword() const;
    const StatementList& getStatementList() const;
    const GenericTerminal& getEndKeyword() const;

    void accept(ParseTreeVisitor& visitor) const override;
};

class InitDeclarator: public Symbol {
    friend class pljit::Parser;

    Identifier identifier;
    GenericTerminal initOperator;
    Literal literal;

    public:
    InitDeclarator();

    const Identifier& getIdentifier() const;
    const GenericTerminal& getInitOperator() const;
    const Literal& getLiteral() const;

    void accept(ParseTreeVisitor& visitor) const override;
};

class InitDeclaratorList: public Symbol {
    friend class pljit::Parser;

    InitDeclarator initDeclarator;
    std::vector<std::tuple<GenericTerminal, InitDeclarator>> additionalInitDeclarators;

    public:
    InitDeclaratorList();

    const InitDeclarator& getInitDeclarator() const;
    const std::vector<std::tuple<GenericTerminal, InitDeclarator>>& getAdditionalInitDeclarators() const;

    void accept(ParseTreeVisitor& visitor) const override;
};

class DeclaratorList: public Symbol {
    friend class pljit::Parser;

    Identifier identifier;
    std::vector<std::tuple<GenericTerminal, Identifier>> additionalIdentifiers;

    public:
    DeclaratorList();

    const Identifier& getIdentifier() const;
    const std::vector<std::tuple<GenericTerminal, Identifier>>& getAdditionalIdentifiers() const;

    void accept(ParseTreeVisitor& visitor) const override;
};

class ConstantDeclarations: public Symbol {
    friend class pljit::Parser;

    GenericTerminal constKeyword;
    InitDeclaratorList initDeclaratorList;
    GenericTerminal semicolon;

    public:
    ConstantDeclarations();

    const GenericTerminal& getConstKeyword() const;
    const InitDeclaratorList& getInitDeclaratorList() const;
    const GenericTerminal& getSemicolon() const;

    void accept(ParseTreeVisitor& visitor) const override;
};

class VariableDeclarations: public Symbol {
    friend class pljit::Parser;

    GenericTerminal varKeyword;
    DeclaratorList declaratorList;
    GenericTerminal semicolon;

    public:
    VariableDeclarations();

    const GenericTerminal& getVarKeyword() const;
    const DeclaratorList& getDeclaratorList() const;
    const GenericTerminal& getSemicolon() const;

    void accept(ParseTreeVisitor& visitor) const override;
};

class ParameterDeclarations: public Symbol {
    friend class pljit::Parser;

    GenericTerminal paramKeyword;
    DeclaratorList declaratorList;
    GenericTerminal semicolon;

    public:
    ParameterDeclarations();

    const GenericTerminal& getParamKeyword() const;
    const DeclaratorList& getDeclaratorList() const;
    const GenericTerminal& getSemicolon() const;

    void accept(ParseTreeVisitor& visitor) const override;
};

class FunctionDefinition: public Symbol {
    friend class pljit::Parser;

    std::optional<ParameterDeclarations> parameterDeclarations;
    std::optional<VariableDeclarations> variableDeclarations;
    std::optional<ConstantDeclarations> constantDeclarations;

    CompoundStatement compoundStatement;

    public:
    FunctionDefinition();

    const std::optional<ParameterDeclarations>& getParameterDeclarations() const;
    const std::optional<VariableDeclarations>& getVariableDeclarations() const;
    const std::optional<ConstantDeclarations>& getConstantDeclarations() const;
    const CompoundStatement& getCompoundStatement() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
} // namespace ParseTree
//---------------------------------------------------------------------------
class Parser { // TODO "RecursiveDescentParser"
    Lexer* lexer;

    public:
    explicit Parser(Lexer& lexer); // TODO manage lexer creation itself?

    Result<ParseTree::FunctionDefinition> parse_program();

    [[nodiscard]] std::optional<SourceCodeError> parseFunctionDefinition(ParseTree::FunctionDefinition& destination);

    [[nodiscard]] std::optional<SourceCodeError> parseParameterDeclarations(std::optional<ParseTree::ParameterDeclarations>& destination);
    [[nodiscard]] std::optional<SourceCodeError> parseVariableDeclarations(std::optional<ParseTree::VariableDeclarations>& destination);
    [[nodiscard]] std::optional<SourceCodeError> parseConstantDeclarations(std::optional<ParseTree::ConstantDeclarations>& destination);

    [[nodiscard]] std::optional<SourceCodeError> parseDeclaratorList(ParseTree::DeclaratorList& destination);
    [[nodiscard]] std::optional<SourceCodeError> parseInitDeclaratorList(ParseTree::InitDeclaratorList& destination);
    [[nodiscard]] std::optional<SourceCodeError> parseInitDeclarator(ParseTree::InitDeclarator& destination);

    [[nodiscard]] std::optional<SourceCodeError> parseCompoundStatement(ParseTree::CompoundStatement& destination);
    [[nodiscard]] std::optional<SourceCodeError> parseStatementList(ParseTree::StatementList& destination);
    [[nodiscard]] std::optional<SourceCodeError> parseStatement(ParseTree::Statement& destination);

    [[nodiscard]] std::optional<SourceCodeError> parseAssignmentExpression(ParseTree::AssignmentExpression& destination);
    [[nodiscard]] std::optional<SourceCodeError> parseAdditiveExpression(ParseTree::AdditiveExpression& destination);
    [[nodiscard]] std::optional<SourceCodeError> parseMultiplicativeExpression(ParseTree::MultiplicativeExpression& destination);

    [[nodiscard]] std::optional<SourceCodeError> parseUnaryExpression(ParseTree::UnaryExpression& destination);
    [[nodiscard]] std::optional<SourceCodeError> parsePrimaryExpression(ParseTree::PrimaryExpression& destination);

    [[nodiscard]] std::optional<SourceCodeError> parseIdentifier(ParseTree::Identifier& destination);
    [[nodiscard]] std::optional<SourceCodeError> parseLiteral(ParseTree::Literal& destination);
    [[nodiscard]] std::optional<SourceCodeError> parseGenericTerminal(
        ParseTree::GenericTerminal& destination,
        Token::TokenType expected_type,
        std::string_view expected_content,
        std::string_view potential_error_message
    );
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_PARSER_HPP
