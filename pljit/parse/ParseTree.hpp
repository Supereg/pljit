//
// Created by Andreas Bauer on 11.08.22.
//

#ifndef PLJIT_PARSETREE_HPP
#define PLJIT_PARSETREE_HPP

#include "../code/SourceCodeManagement.hpp"
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
    friend class pljit::parse::Parser;

    protected:
    // TODO not initialized in many symbols!!! (e.g. Primary Expression!)
    // TODO getter checks for nullptr!!!
    code::SourceCodeReference src_reference;
    public:
    Symbol();
    Symbol(code::SourceCodeReference src_reference);

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

    std::string_view value() const;
    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class Literal : public Symbol {
    friend class pljit::parse::Parser;
    long long literalValue; // 64-bit integer

    public:
    Literal();
    Literal(code::SourceCodeReference src_reference, long long literalValue);

    long long value() const;
    std::string_view string_value() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class PrimaryExpression : public Symbol {
    friend class pljit::parse::Parser;

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
//---------------------------------------------------------------------------
class UnaryExpression : public Symbol {
    friend class pljit::parse::Parser; // TODO can we avoid all the friend definitions in this file?

    std::optional<GenericTerminal> unaryOperator;
    PrimaryExpression primaryExpression;

    public:
    UnaryExpression();

    const std::optional<GenericTerminal>& getUnaryOperator() const;
    const PrimaryExpression& getPrimaryExpression() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class MultiplicativeExpression : public Symbol {
    friend class pljit::parse::Parser;

    UnaryExpression expression;
    std::vector<std::tuple<GenericTerminal, MultiplicativeExpression>> optionalOperand;

    public:
    MultiplicativeExpression();

    const UnaryExpression& getExpression() const;
    std::optional<std::tuple<const GenericTerminal&, const MultiplicativeExpression&>> getOperand() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class AdditiveExpression : public Symbol {
    friend class pljit::parse::Parser;

    MultiplicativeExpression expression;

    std::vector<std::tuple<GenericTerminal, AdditiveExpression>> optionalOperand;

    public:
    AdditiveExpression();

    const MultiplicativeExpression& getExpression() const;
    std::optional<std::tuple<const GenericTerminal&, const AdditiveExpression&>> getOperand() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class AssignmentExpression : public Symbol {
    friend class pljit::parse::Parser;

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
//---------------------------------------------------------------------------
class Statement : public Symbol {
    friend class pljit::parse::Parser;

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
//---------------------------------------------------------------------------
class StatementList : public Symbol {
    friend class pljit::parse::Parser;

    Statement statement;
    std::vector<std::tuple<GenericTerminal, Statement>> additionalStatements;

    public:
    StatementList();

    const Statement& getStatement() const;
    const std::vector<std::tuple<GenericTerminal, Statement>>& getAdditionalStatements() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class CompoundStatement : public Symbol {
    friend class pljit::parse::Parser;

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
//---------------------------------------------------------------------------
class InitDeclarator : public Symbol {
    friend class pljit::parse::Parser;

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
//---------------------------------------------------------------------------
class InitDeclaratorList : public Symbol {
    friend class pljit::parse::Parser;

    InitDeclarator initDeclarator;
    std::vector<std::tuple<GenericTerminal, InitDeclarator>> additionalInitDeclarators;

    public:
    InitDeclaratorList();

    const InitDeclarator& getInitDeclarator() const;
    const std::vector<std::tuple<GenericTerminal, InitDeclarator>>& getAdditionalInitDeclarators() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class DeclaratorList : public Symbol {
    friend class pljit::parse::Parser;

    Identifier identifier;
    std::vector<std::tuple<GenericTerminal, Identifier>> additionalIdentifiers;

    public:
    DeclaratorList();

    const Identifier& getIdentifier() const;
    const std::vector<std::tuple<GenericTerminal, Identifier>>& getAdditionalIdentifiers() const;

    void accept(ParseTreeVisitor& visitor) const override;
};
//---------------------------------------------------------------------------
class ConstantDeclarations : public Symbol {
    friend class pljit::parse::Parser;

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
//---------------------------------------------------------------------------
class VariableDeclarations : public Symbol {
    friend class pljit::parse::Parser;

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
//---------------------------------------------------------------------------
class ParameterDeclarations : public Symbol {
    friend class pljit::parse::Parser;

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
//---------------------------------------------------------------------------
class FunctionDefinition : public Symbol {
    friend class pljit::parse::Parser;

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
} // namespace pljit::parse
//---------------------------------------------------------------------------

#endif //PLJIT_PARSETREE_HPP