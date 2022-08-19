//
// Created by Andreas Bauer on 11.08.22.
//

#ifndef PLJIT_PARSETREE_HPP
#define PLJIT_PARSETREE_HPP

#include "../code/SourceCode.hpp"
#include <memory>
#include <optional>
#include <vector>

//---------------------------------------------------------------------------
namespace pljit::parse {
//---------------------------------------------------------------------------
class Parser;
//---------------------------------------------------------------------------
class ParseTreeVisitor;
class AdditiveExpression;
//---------------------------------------------------------------------------
class Symbol {
    protected:
    code::SourceCodeReference src_reference;
    public:
    Symbol();
    explicit Symbol(code::SourceCodeReference src_reference);

    virtual ~Symbol() = default;

    const code::SourceCodeReference& reference() const;

    virtual void accept(ParseTreeVisitor& visitor) const = 0;
};
//---------------------------------------------------------------------------
class GenericTerminal : public Symbol {
    public:
    GenericTerminal();
    explicit GenericTerminal(code::SourceCodeReference src_reference);

    std::string_view value() const;
    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class Identifier : public Symbol {
    public:
    Identifier();
    explicit Identifier(code::SourceCodeReference src_reference);

    std::string_view value() const;
    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class Literal : public Symbol {
    long long literalValue; // 64-bit integer

    public:
    Literal();
    Literal(code::SourceCodeReference src_reference, long long literalValue);

    long long value() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class PrimaryExpression : public Symbol {
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
    explicit PrimaryExpression(Identifier identifier);
    explicit PrimaryExpression(Literal literal);
    PrimaryExpression(GenericTerminal open, AdditiveExpression additiveExpression, GenericTerminal close);

    Type getType() const;

    const Identifier& asIdentifier() const;
    const Literal& asLiteral() const;
    std::tuple<const GenericTerminal&, const AdditiveExpression&, const GenericTerminal&> asBracketedExpression() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class UnaryExpression : public Symbol {
    std::optional<GenericTerminal> unaryOperator;
    PrimaryExpression primaryExpression;

    public:
    UnaryExpression();
    explicit UnaryExpression(PrimaryExpression primaryExpression);
    UnaryExpression(GenericTerminal unaryOperator, PrimaryExpression primaryExpression);

    const std::optional<GenericTerminal>& getUnaryOperator() const;
    const PrimaryExpression& getPrimaryExpression() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class MultiplicativeExpression : public Symbol {
    UnaryExpression expression;
    std::vector<std::tuple<GenericTerminal, MultiplicativeExpression>> optionalOperand;

    public:
    MultiplicativeExpression();
    explicit MultiplicativeExpression(UnaryExpression unaryExpression);
    MultiplicativeExpression(UnaryExpression unaryExpression, GenericTerminal op, MultiplicativeExpression multiplicativeExpression);

    const UnaryExpression& getExpression() const;
    std::optional<std::tuple<const GenericTerminal&, const MultiplicativeExpression&>> getOperand() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class AdditiveExpression : public Symbol {
    MultiplicativeExpression expression;

    std::vector<std::tuple<GenericTerminal, AdditiveExpression>> optionalOperand;

    public:
    AdditiveExpression();
    explicit AdditiveExpression(MultiplicativeExpression multiplicativeExpression);
    AdditiveExpression(MultiplicativeExpression multiplicativeExpression, GenericTerminal op, AdditiveExpression additiveExpression);

    const MultiplicativeExpression& getExpression() const;
    std::optional<std::tuple<const GenericTerminal&, const AdditiveExpression&>> getOperand() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class AssignmentExpression : public Symbol {
    Identifier identifier;
    GenericTerminal assignmentOperator;
    AdditiveExpression additiveExpression;

    public:
    AssignmentExpression();
    AssignmentExpression(Identifier identifier, GenericTerminal op, AdditiveExpression additiveExpression);

    const Identifier& getIdentifier() const;
    const GenericTerminal& getAssignmentOperator() const;
    const AdditiveExpression& getAdditiveExpression() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class Statement : public Symbol {
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
    explicit Statement(AssignmentExpression assignmentExpression);
    Statement(GenericTerminal returnKeyword, AdditiveExpression additiveExpression);

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
//---------------------------------------------------------------------------
class StatementList : public Symbol {
    Statement statement;
    std::vector<std::tuple<GenericTerminal, Statement>> additionalStatements;

    public:
    StatementList();
    explicit StatementList(Statement statement);

    void appendStatement(GenericTerminal separator, Statement statement);

    const Statement& getStatement() const;
    const std::vector<std::tuple<GenericTerminal, Statement>>& getAdditionalStatements() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class CompoundStatement : public Symbol {
    GenericTerminal beginKeyword;
    StatementList statementList;
    GenericTerminal endKeyword;

    public:
    CompoundStatement();
    CompoundStatement(GenericTerminal beginKeyword, StatementList statementList, GenericTerminal endKeyword);

    const GenericTerminal& getBeginKeyword() const;
    const StatementList& getStatementList() const;
    const GenericTerminal& getEndKeyword() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class InitDeclarator : public Symbol {
    Identifier identifier;
    GenericTerminal initOperator;
    Literal literal;

    public:
    InitDeclarator();
    InitDeclarator(Identifier identifier, GenericTerminal initOperator, Literal literal);

    const Identifier& getIdentifier() const;
    const GenericTerminal& getInitOperator() const;
    const Literal& getLiteral() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class InitDeclaratorList : public Symbol {
    InitDeclarator initDeclarator;
    std::vector<std::tuple<GenericTerminal, InitDeclarator>> additionalInitDeclarators;

    public:
    InitDeclaratorList();
    explicit InitDeclaratorList(InitDeclarator initDeclarator);

    void appendInitDeclarator(GenericTerminal separator, InitDeclarator initDeclarator);

    const InitDeclarator& getInitDeclarator() const;
    const std::vector<std::tuple<GenericTerminal, InitDeclarator>>& getAdditionalInitDeclarators() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class DeclaratorList : public Symbol {
    Identifier identifier;
    std::vector<std::tuple<GenericTerminal, Identifier>> additionalIdentifiers;

    public:
    DeclaratorList();
    explicit DeclaratorList(Identifier identifier);

    void appendIdentifier(GenericTerminal separator, Identifier identifier);

    const Identifier& getIdentifier() const;
    const std::vector<std::tuple<GenericTerminal, Identifier>>& getAdditionalIdentifiers() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class ConstantDeclarations : public Symbol {
    GenericTerminal constKeyword;
    InitDeclaratorList initDeclaratorList;
    GenericTerminal semicolon;

    public:
    ConstantDeclarations();
    ConstantDeclarations(GenericTerminal constKeyword, InitDeclaratorList initDeclaratorList, GenericTerminal semicolon);

    const GenericTerminal& getConstKeyword() const;
    const InitDeclaratorList& getInitDeclaratorList() const;
    const GenericTerminal& getSemicolon() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class VariableDeclarations : public Symbol {
    GenericTerminal varKeyword;
    DeclaratorList declaratorList;
    GenericTerminal semicolon;

    public:
    VariableDeclarations();
    VariableDeclarations(GenericTerminal varKeyword, DeclaratorList declaratorList, GenericTerminal semicolon);

    const GenericTerminal& getVarKeyword() const;
    const DeclaratorList& getDeclaratorList() const;
    const GenericTerminal& getSemicolon() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class ParameterDeclarations : public Symbol {
    GenericTerminal paramKeyword;
    DeclaratorList declaratorList;
    GenericTerminal semicolon;

    public:
    ParameterDeclarations();
    ParameterDeclarations(GenericTerminal paramKeyword, DeclaratorList declaratorList, GenericTerminal semicolon);

    const GenericTerminal& getParamKeyword() const;
    const DeclaratorList& getDeclaratorList() const;
    const GenericTerminal& getSemicolon() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class FunctionDefinition : public Symbol {
    std::optional<ParameterDeclarations> parameterDeclarations;
    std::optional<VariableDeclarations> variableDeclarations;
    std::optional<ConstantDeclarations> constantDeclarations;

    CompoundStatement compoundStatement;

    GenericTerminal terminator;

    public:
    FunctionDefinition();
    FunctionDefinition(
        std::optional<ParameterDeclarations> parameterDeclarations,
        std::optional<VariableDeclarations> variableDeclarations,
        std::optional<ConstantDeclarations> constantDeclarations,
        CompoundStatement compoundStatement,
        GenericTerminal terminator
    );

    const std::optional<ParameterDeclarations>& getParameterDeclarations() const;
    const std::optional<VariableDeclarations>& getVariableDeclarations() const;
    const std::optional<ConstantDeclarations>& getConstantDeclarations() const;
    const CompoundStatement& getCompoundStatement() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
} // namespace pljit::parse
//---------------------------------------------------------------------------

#endif //PLJIT_PARSETREE_HPP