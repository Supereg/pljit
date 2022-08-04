//
// Created by Andreas Bauer on 30.07.22.
//

#ifndef PLJIT_AST_HPP
#define PLJIT_AST_HPP
#include "SymbolTable.hpp"
#include "ASTVisitor.hpp"
#include <vector>
#include <memory>
#include <optional>

//---------------------------------------------------------------------------
namespace pljit::ast {
//---------------------------------------------------------------------------
class Node {
    public:
    enum class Type {
        // EXPRESSIONS
        LITERAL,
        VARIABLE,

        // UNARY EXPRESSIONS
        UNARY_PLUS,
        UNARY_MINUS,

        // BINARY EXPRESSIONS
        ADD,
        SUBTRACT,
        MULTIPLY,
        DIVIDE,

        // STATEMENTS
        ASSIGNMENT_STATEMENT,
        RETURN_STATEMENT,

        // DECLARATIONS
        PARAM_DECLARATION,
        VAR_DECLARATION,
        CONST_DECLARATION,

        FUNCTION,
    };

    Node() = default;
    virtual ~Node() = default;

    virtual Type getType() const = 0;
    virtual void accept(ASTVisitor& visitor) const = 0;
};


class Expression: public Node {};

class Literal: public Expression {
    long long literal_value;
    public:
    explicit Literal(long long literal_value);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;

    long long int value() const;
};

class Variable: public Expression {
    symbol_id symbolId;
    std::string_view name;

    public:
    explicit Variable(symbol_id symbolId, std::string_view name);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;

    symbol_id getSymbolId() const;
    const std::string_view& getName() const;
};

class UnaryExpression: public Expression {
    protected:
    std::unique_ptr<Expression> child;

    public:
    explicit UnaryExpression(std::unique_ptr<Expression> child);

    const Expression& getChild() const;
};

class BinaryExpression: public Expression {
    protected:
    std::unique_ptr<Expression> leftChild;
    std::unique_ptr<Expression> rightChild;

    public:
    BinaryExpression(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild);

    const Expression& getLeft() const;
    const Expression& getRight() const;
};

class UnaryPlus: public UnaryExpression {
    public:
    explicit UnaryPlus(std::unique_ptr<Expression> child);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
};

class UnaryMinus: public UnaryExpression {
    public:
    explicit UnaryMinus(std::unique_ptr<Expression> child);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
};

class Add: public BinaryExpression {
    public:
    Add(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
};

class Subtract: public BinaryExpression {
    public:
    Subtract(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
};

class Multiply: public BinaryExpression {
    public:
    Multiply(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
};

class Divide: public BinaryExpression {
    public:
    Divide(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
};

class Statement: public Node {
    protected:
    std::unique_ptr<Expression> expression;

    public:
    explicit Statement(std::unique_ptr<Expression> expression);

    const Expression& getExpression() const;
};

class AssignmentStatement: public Statement {
    Variable variable;

    public:
    AssignmentStatement(std::unique_ptr<Expression> expression, Variable variable);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;

    const Variable& getVariable() const;
};

class ReturnStatement: public Statement {
    public:
    explicit ReturnStatement(std::unique_ptr<Expression> expression);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
};

class Declaration: public Node {
    protected:
    std::vector<Variable> declaredIdentifiers;
    public:
    Declaration();
    explicit Declaration(std::vector<Variable> declaredIdentifiers);

    const std::vector<Variable>& getDeclaredIdentifiers() const;
};

class ParamDeclaration: public Declaration {
    public:
    ParamDeclaration();
    explicit ParamDeclaration(std::vector<Variable> declaredIdentifiers);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
};

class VarDeclaration: public Declaration {
    public:
    VarDeclaration();
    explicit VarDeclaration(std::vector<Variable> declaredIdentifiers);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
};

class ConstDeclaration: public Declaration {
    std::vector<Literal> literalValues;
    public:
    ConstDeclaration();
    ConstDeclaration(std::vector<Variable> declaredIdentifiers, std::vector<Literal> literalValues);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;

    std::vector<std::tuple<const Variable&, const Literal&>> getConstDeclarations() const;
};

class Function: public Node {
    // TODO really friends?
    friend class ASTBuilder;

    std::optional<ParamDeclaration> paramDeclaration;
    std::optional<VarDeclaration> varDeclaration;
    std::optional<ConstDeclaration> constDeclaration;

    std::vector<std::unique_ptr<Statement>> statements;

    public:
    Function();

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;

    const std::optional<ParamDeclaration>& getParamDeclaration() const;
    const std::optional<VarDeclaration>& getVarDeclaration() const;
    const std::optional<ConstDeclaration>& getConstDeclaration() const;
    const std::vector<std::unique_ptr<Statement>>& getStatements() const;
};
//---------------------------------------------------------------------------
} // namespace pljit::ast
//---------------------------------------------------------------------------

#endif //PLJIT_AST_HPP
