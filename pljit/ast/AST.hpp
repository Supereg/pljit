//
// Created by Andreas Bauer on 30.07.22.
//

#ifndef PLJIT_AST_HPP
#define PLJIT_AST_HPP

#include "./ASTVisitor.hpp"
#include "pljit/SymbolTable.hpp"
#include "pljit/EvaluationContext.hpp"
#include <memory>
#include <optional>
#include <vector>

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


class Expression: public Node {
    public:
    virtual Result<long long> evaluate(EvaluationContext& context) const = 0;
};

class Literal: public Expression {
    long long literal_value;
    public:
    explicit Literal(long long literal_value);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
    Result<long long> evaluate(EvaluationContext& context) const override;

    long long value() const;
};

class Variable: public Expression {
    symbol_id symbolId;
    std::string_view name;

    public:
    explicit Variable(symbol_id symbolId, std::string_view name);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
    Result<long long> evaluate(EvaluationContext& context) const override;

    symbol_id getSymbolId() const;
    const std::string_view& getName() const;
};

class UnaryExpression: public Expression {
    protected:
    std::unique_ptr<Expression> child;

    public:
    explicit UnaryExpression(std::unique_ptr<Expression> child);

    const Expression& getChild() const;
    std::unique_ptr<Expression>& getChildPtr();
};

class BinaryExpression: public Expression {
    protected:
    std::unique_ptr<Expression> leftChild;
    std::unique_ptr<Expression> rightChild;

    public:
    BinaryExpression(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild);

    const Expression& getLeft() const;
    const Expression& getRight() const;

    std::unique_ptr<Expression>& getLeftPtr();
    std::unique_ptr<Expression>& getRightPtr();
};

class UnaryPlus: public UnaryExpression {
    public:
    explicit UnaryPlus(std::unique_ptr<Expression> child);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
    Result<long long> evaluate(EvaluationContext& context) const override;
};

class UnaryMinus: public UnaryExpression {
    public:
    explicit UnaryMinus(std::unique_ptr<Expression> child);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
    Result<long long> evaluate(EvaluationContext& context) const override;
};

class Add: public BinaryExpression {
    public:
    Add(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
    Result<long long> evaluate(EvaluationContext& context) const override;
};

class Subtract: public BinaryExpression {
    public:
    Subtract(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
    Result<long long> evaluate(EvaluationContext& context) const override;
};

class Multiply: public BinaryExpression {
    public:
    Multiply(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
    Result<long long> evaluate(EvaluationContext& context) const override;
};

class Divide: public BinaryExpression {
    code::SourceCodeReference operatorSymbol;
    public:
    Divide(std::unique_ptr<Expression> leftChild, std::unique_ptr<Expression> rightChild, code::SourceCodeReference operatorSymbol);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
    Result<long long> evaluate(EvaluationContext& context) const override;
};

class Statement: public Node {
    protected:
    std::unique_ptr<Expression> expression;

    public:
    explicit Statement(std::unique_ptr<Expression> expression);

    const Expression& getExpression() const;
    std::unique_ptr<Expression>& getExpressionPtr();


    [[nodiscard]] virtual std::optional<code::SourceCodeError> evaluate(EvaluationContext& context) const = 0;
};

class AssignmentStatement: public Statement {
    Variable variable;

    public:
    AssignmentStatement(std::unique_ptr<Expression> expression, Variable variable);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
    [[nodiscard]] std::optional<code::SourceCodeError> evaluate(EvaluationContext& context) const override;

    const Variable& getVariable() const;
};

class ReturnStatement: public Statement {
    public:
    explicit ReturnStatement(std::unique_ptr<Expression> expression);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
    [[nodiscard]] std::optional<code::SourceCodeError> evaluate(EvaluationContext& context) const override;
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
    code::SourceCodeReference paramKeyword;

    public:
    ParamDeclaration();
    explicit ParamDeclaration(code::SourceCodeReference paramKeyword, std::vector<Variable> declaredIdentifiers);

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
    [[nodiscard]] std::optional<code::SourceCodeError> evaluate(EvaluationContext& context, std::vector<long long> arguments) const;
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
    void evaluate(EvaluationContext& context) const;

    std::vector<std::tuple<const Variable&, const Literal&>> getConstDeclarations() const;
};

class Function: public Node {
    // TODO really friends?
    friend class ASTBuilder;

    std::optional<ParamDeclaration> paramDeclaration;
    std::optional<VarDeclaration> varDeclaration;
    std::optional<ConstDeclaration> constDeclaration;

    std::vector<std::unique_ptr<Statement>> statements;

    code::SourceCodeReference begin_reference;
    std::size_t total_symbols;

    public:
    Function();

    // Move Construction
    Function(Function&& other) noexcept = default;
    // Move Assignment
    Function& operator=(Function&& other) noexcept = default;

    Type getType() const override;
    void accept(ASTVisitor& visitor) const override;
    [[nodiscard]] Result<long long> evaluate(const std::vector<long long>& arguments) const;

    const std::optional<ParamDeclaration>& getParamDeclaration() const;
    const std::optional<VarDeclaration>& getVarDeclaration() const;
    const std::optional<ConstDeclaration>& getConstDeclaration() const;
    const std::vector<std::unique_ptr<Statement>>& getStatements() const;
    std::vector<std::unique_ptr<Statement>>& getStatements();

    std::size_t symbol_count() const;
};
//---------------------------------------------------------------------------
} // namespace pljit::ast
//---------------------------------------------------------------------------

#endif //PLJIT_AST_HPP
