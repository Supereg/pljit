//
// Created by Andreas Bauer on 31.07.22.
//

#include "./ASTBuilder.hpp"
#include "./AST.hpp"
#include "pljit/parse/ParseTree.hpp"
#include "pljit/code/SourceCode.hpp"
#include "pljit/lang.hpp"

//---------------------------------------------------------------------------
namespace pljit::ast {
//---------------------------------------------------------------------------
ASTBuilder::ASTBuilder() : symbolTable() {}

Result<Function> ASTBuilder::analyzeFunction(const parse::FunctionDefinition& node) {
    Result<std::unique_ptr<Statement>> result;

    std::optional<ParamDeclaration> paramDeclaration;
    std::optional<VarDeclaration> varDeclaration;
    std::optional<ConstDeclaration> constDeclaration;
    std::vector<std::unique_ptr<Statement>> statements;

    if (node.getParameterDeclarations()) {
        auto declResult = analyzeParamDeclaration(*node.getParameterDeclarations());
        if (!declResult) {
            return declResult.error();
        }

        paramDeclaration = declResult.release();
    }

    if (node.getVariableDeclarations()) {
        auto declResult = analyzeVarDeclaration(*node.getVariableDeclarations());
        if (!declResult) {
            return declResult.error();
        }
        varDeclaration = declResult.release();
    }

    if (node.getConstantDeclarations()) {
        auto declResult = analyzeConstDeclaration(*node.getConstantDeclarations());
        if (!declResult) {
            return declResult.error();
        }
        constDeclaration = declResult.release();
    }


    auto& compound = node.getCompoundStatement();
    auto& statementList = compound.getStatementList();

    result = analyzeStatement(statementList.getStatement());
    if (!result) {
        return result.error();
    }

    statements.push_back(result.release());

    for (auto& [genericTerminal, statement]: statementList.getAdditionalStatements()) {
        result = analyzeStatement(statement);
        if (!result) {
            return result.error();
        }

        statements.push_back(result.release());
    }

    Function function{
        std::move(paramDeclaration),
        std::move(varDeclaration),
        std::move(constDeclaration),
        std::move(statements),
        symbolTable.size()
    };

    bool found_return = false;
    for (auto& statement: function.getStatements()) {
        if (statement->getType() == Node::Type::RETURN_STATEMENT) {
            found_return = true;
            break;
        }
    }

    if (!found_return) {
        return compound.getEndKeyword().reference()
            .makeError(code::ErrorType::ERROR, "Reached end of function without a RETURN statement!");
    }

    return function;
}

Result<ParamDeclaration> ASTBuilder::analyzeParamDeclaration(const parse::ParameterDeclarations& node) {
    Result<symbol_id> result;
    auto& declaratorList = node.getDeclaratorList();

    std::vector<Variable> variables;
    variables.reserve(1 + declaratorList.getAdditionalIdentifiers().size());

    result = symbolTable.declareIdentifier(declaratorList.getIdentifier(), SymbolTable::SymbolType::PARAM);
    if (!result) {
        return result.error();
    }

    variables.emplace_back(result.release(), declaratorList.getIdentifier().value());

    for (auto& [genericTerminal, identifier]: declaratorList.getAdditionalIdentifiers()) {
        result = symbolTable.declareIdentifier(identifier, SymbolTable::SymbolType::PARAM);
        if (!result) {
            return result.error();
        }

        variables.emplace_back(result.release(), identifier.value());
    }

    return ParamDeclaration{ variables };
}

Result<VarDeclaration> ASTBuilder::analyzeVarDeclaration(const parse::VariableDeclarations& node) {
    Result<symbol_id> result;
    auto& declaratorList = node.getDeclaratorList();

    std::vector<Variable> variables;
    variables.reserve(1 + declaratorList.getAdditionalIdentifiers().size());

    result = symbolTable.declareIdentifier(declaratorList.getIdentifier(), SymbolTable::SymbolType::VAR);
    if (!result) {
        return result.error();
    }

    variables.emplace_back(result.release(), declaratorList.getIdentifier().value());

    for (auto& [genericTerminal, identifier]: declaratorList.getAdditionalIdentifiers()) {
        result = symbolTable.declareIdentifier(identifier, SymbolTable::SymbolType::VAR);
        if (!result) {
            return result.error();
        }

        variables.emplace_back(result.release(), identifier.value());
    }

    return VarDeclaration{ variables };
}

Result<ConstDeclaration> ASTBuilder::analyzeConstDeclaration(const parse::ConstantDeclarations& node) {
    Result<symbol_id> result;
    auto& initDeclaratorList = node.getInitDeclaratorList();
    auto& initDeclarator = initDeclaratorList.getInitDeclarator();

    std::vector<Variable> variables;
    std::vector<Literal> literals;
    variables.reserve(1 + initDeclaratorList.getAdditionalInitDeclarators().size());
    literals.reserve(1 + initDeclaratorList.getAdditionalInitDeclarators().size());

    result = symbolTable.declareIdentifier(initDeclarator.getIdentifier(), SymbolTable::SymbolType::CONST);
    if (!result) {
        return result.error();
    }

    variables.emplace_back(result.release(), initDeclarator.getIdentifier().value());
    literals.emplace_back(initDeclarator.getLiteral().value());

    for (auto& [genericTerminal, declarator]: initDeclaratorList.getAdditionalInitDeclarators()) {
        result = symbolTable.declareIdentifier(declarator.getIdentifier(), SymbolTable::SymbolType::CONST);
        if (!result) {
            return result.error();
        }

        variables.emplace_back(result.release(), declarator.getIdentifier().value());
        literals.emplace_back(declarator.getLiteral().value());
    }

    return ConstDeclaration{ variables, literals };
}

Result<std::unique_ptr<Statement>> ASTBuilder::analyzeStatement(const parse::Statement& node) {
    Result<std::unique_ptr<Expression>> result;

    switch (node.getType()) {
        case parse::Statement::Type::ASSIGNMENT: {
            auto& assignment = node.asAssignmentExpression();

            result = analyzeExpression(assignment.getAdditiveExpression());
            if (!result) {
                return result.error();
            }

            Result<symbol_id> target = symbolTable.useAsAssignmentTarget(assignment.getIdentifier());
            if (!target) {
                return target.error();
            }

            std::unique_ptr<Statement> statement = std::make_unique<AssignmentStatement>(
                result.release(),
                Variable{ target.release(), assignment.getIdentifier().value() }
            );
            return statement;
        }
        case parse::Statement::Type::RETURN: {
            auto [returnKeyword, additiveExpression] = node.asReturnExpression();

            result = analyzeExpression(additiveExpression);
            if (!result) {
                return result.error();
            }

            std::unique_ptr<Statement> statement = std::make_unique<ReturnStatement>(result.release());
            return statement;
        }
        case parse::Statement::Type::NONE:
            return node.reference()
                .makeError(code::ErrorType::ERROR, "Encountered illegal parser tree state! Expected ASSIGNMENT or RETURN!");
    }

    return {};
}
Result<std::unique_ptr<Expression>> ASTBuilder::analyzeExpression(const parse::AdditiveExpression& node) {
    Result<std::unique_ptr<Expression>> result;

    result = analyzeExpression(node.getExpression());
    if (!node.getOperand()) {
        return result;
    }

    if (!result) {
        return result.error();
    }

    std::unique_ptr<Expression> multiplicativeExpression = result.release();
    auto [operatorTerminal, additiveExpression] = *node.getOperand();

    result = analyzeExpression(additiveExpression);
    if (!result) {
        return result.error();
    }

    if (operatorTerminal.value() == Operator::PLUS) {
        std::unique_ptr<Expression> expression = std::make_unique<Add>(std::move(multiplicativeExpression), result.release());
        return expression;
    } else if (operatorTerminal.value() == Operator::MINUS) {
        std::unique_ptr<Expression> expression = std::make_unique<Subtract>(std::move(multiplicativeExpression), result.release());
        return expression;
    } else {
        return node.reference()
            .makeError(code::ErrorType::ERROR, "Encountered illegal parse tree state! Expected PLUS or MINUS!");
    }
}
Result<std::unique_ptr<Expression>> ASTBuilder::analyzeExpression(const parse::MultiplicativeExpression& node) {
    Result<std::unique_ptr<Expression>> result;

    result = analyzeExpression(node.getExpression());
    if (!node.getOperand()) {
        return result;
    }

    if (!result) {
        return result.error();
    }

    std::unique_ptr<Expression> unaryExpression = result.release();
    auto [operatorTerminal, multiplicativeExpression] = *node.getOperand();

    result = analyzeExpression(multiplicativeExpression);
    if (!result) {
        return result.error();
    }

    if (operatorTerminal.value() == Operator::MULTIPLICATION) {
        std::unique_ptr<Expression> expression = std::make_unique<Multiply>(std::move(unaryExpression), result.release());
        return expression;
    } else if (operatorTerminal.value() == Operator::DIVISION) {
        std::unique_ptr<Expression> expression = std::make_unique<Divide>(std::move(unaryExpression), result.release());
        return expression;
    } else {
        return node.reference()
            .makeError(code::ErrorType::ERROR, "Encountered illegal parse tree state! Expected MULTIPLICATION or DIVISION!");
    }
}
Result<std::unique_ptr<Expression>> ASTBuilder::analyzeExpression(const parse::UnaryExpression& node) {
    Result<std::unique_ptr<Expression>> result;

    result = analyzeExpression(node.getPrimaryExpression());
    if (!node.getUnaryOperator()) {
        return result;
    }

    if (!result) {
        return result.error();
    }

    auto& operatorTerminal = *node.getUnaryOperator();

    if (operatorTerminal.value() == Operator::PLUS) {
        std::unique_ptr<Expression> expression = std::make_unique<UnaryPlus>(result.release());
        return expression;
    } else if (operatorTerminal.value() == Operator::MINUS) {
        std::unique_ptr<Expression> expression = std::make_unique<UnaryMinus>(result.release());
        return expression;
    } else {
        return node.reference()
            .makeError(code::ErrorType::ERROR, "Encountered illegal parse tree state! Expected PLUS or MINUS!");
    }
}
Result<std::unique_ptr<Expression>> ASTBuilder::analyzeExpression(const parse::PrimaryExpression& node) {
    std::unique_ptr<Expression> expression;

    switch (node.getType()) {
        case parse::PrimaryExpression::Type::IDENTIFIER: {
            Result<symbol_id> result = symbolTable.useIdentifier(node.asIdentifier());
            if (!result) {
                return result.error();
            }

            expression = std::make_unique<Variable>(result.release(), node.asIdentifier().value());
            return expression;
        }
        case parse::PrimaryExpression::Type::LITERAL:
            expression = std::make_unique<Literal>(node.asLiteral().value());
            return expression;
        case parse::PrimaryExpression::Type::ADDITIVE_EXPRESSION: {
            auto [openParenthesis, additiveExpression, closeParenthesis] = node.asBracketedExpression();
            return analyzeExpression(additiveExpression);
        }
        default:
            return node.reference()
                .makeError(code::ErrorType::ERROR, "Encountered illegal parse tree state! Expected IDENTIFIER, LITERAL or ADDITIVE_EXPRESSION!");
    }
}
//---------------------------------------------------------------------------
} // namespace pljit::ast
//---------------------------------------------------------------------------
