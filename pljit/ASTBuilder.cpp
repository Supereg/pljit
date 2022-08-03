//
// Created by Andreas Bauer on 31.07.22.
//

#include "ASTBuilder.hpp"
#include "AST.hpp"
#include "Parser.hpp" // TODO ParseTree header!

//---------------------------------------------------------------------------
namespace pljit::ast {
//---------------------------------------------------------------------------
ASTBuilder::ASTBuilder() : symbolTable() {}

Result<Function> ASTBuilder::analyzeFunction(const ParseTree::FunctionDefinition& node) {
    Function function;
    Result<std::unique_ptr<Statement>> result;

    if (node.getParameterDeclarations()) {
        auto declResult = analyzeParamDeclaration(*node.getParameterDeclarations());
        if (declResult.failure()) {
            return declResult.error();
        }
        function.paramDeclaration = *declResult;
    }

    if (node.getVariableDeclarations()) {
        auto declResult = analyzeVarDeclaration(*node.getVariableDeclarations());
        if (declResult.failure()) {
            return declResult.error();
        }
        function.varDeclaration = *declResult;
    }

    if (node.getConstantDeclarations()) {
        auto declResult = analyzeConstDeclaration(*node.getConstantDeclarations());
        if (declResult.failure()) {
            return declResult.error();
        }
        function.constDeclaration = *declResult;
    }


    auto& compound = node.getCompoundStatement();
    auto& statementList = compound.getStatementList();

    result = analyzeStatement(statementList.getStatement());
    if (result.failure()) {
        return result.error();
    }

    function.statements.push_back(result.release());

    for (auto& [genericTerminal, statement]: statementList.getAdditionalStatements()) {
        result = analyzeStatement(statement);
        if (result.failure()) {
            return result.error();
        }

        function.statements.push_back(result.release());
    }

    return function;
}

Result<ParamDeclaration> ASTBuilder::analyzeParamDeclaration(const ParseTree::ParameterDeclarations& node) {
    Result<symbol_id> result;
    auto& declaratorList = node.getDeclaratorList();

    std::vector<Variable> variables;
    variables.reserve(1 + declaratorList.getAdditionalIdentifiers().size());

    // TODO parameters aren't constants right?
    result = symbolTable.declareIdentifier(declaratorList.getIdentifier(), SymbolTable::SymbolType::PARAM);
    if (result.failure()) {
        return result.error();
    }

    variables.emplace_back(*result);

    for (auto& [genericTerminal, identifier]: declaratorList.getAdditionalIdentifiers()) {
        result = symbolTable.declareIdentifier(identifier, SymbolTable::SymbolType::PARAM);
        if (result.failure()) {
            return result.error();
        }

        variables.emplace_back(*result);
    }

    return ParamDeclaration{ variables };
}

Result<VarDeclaration> ASTBuilder::analyzeVarDeclaration(const ParseTree::VariableDeclarations& node) {
    // TODO code duplication!
    Result<symbol_id> result;
    auto& declaratorList = node.getDeclaratorList();

    std::vector<Variable> variables;
    variables.reserve(1 + declaratorList.getAdditionalIdentifiers().size());

    result = symbolTable.declareIdentifier(declaratorList.getIdentifier(), SymbolTable::SymbolType::VAR);
    if (result.failure()) {
        return result.error();
    }

    variables.emplace_back(*result);

    for (auto& [genericTerminal, identifier]: declaratorList.getAdditionalIdentifiers()) {
        result = symbolTable.declareIdentifier(identifier, SymbolTable::SymbolType::CONST);
        if (result.failure()) {
            return result.error();
        }

        variables.emplace_back(*result);
    }

    return VarDeclaration{ variables };
}

Result<ConstDeclaration> ASTBuilder::analyzeConstDeclaration(const ParseTree::ConstantDeclarations& node) {
    Result<symbol_id> result;
    auto& initDeclaratorList = node.getInitDeclaratorList();

    std::vector<Variable> variables;
    std::vector<Literal> literals;
    variables.reserve(1 + initDeclaratorList.getAdditionalInitDeclarators().size());
    literals.reserve(1 + initDeclaratorList.getAdditionalInitDeclarators().size());

    result = symbolTable.declareIdentifier(initDeclaratorList.getInitDeclarator().getIdentifier(), SymbolTable::SymbolType::CONST);
    if (result.failure()) {
        return result.error();
    }

    variables.emplace_back(*result);
    literals.emplace_back(initDeclaratorList.getInitDeclarator().getLiteral().value());

    for (auto& [genericTerminal, declarator]: initDeclaratorList.getAdditionalInitDeclarators()) {
        result = symbolTable.declareIdentifier(declarator.getIdentifier(), SymbolTable::SymbolType::CONST);
        if (result.failure()) {
            return result.error();
        }

        variables.emplace_back(*result);
        literals.emplace_back(declarator.getLiteral().value());
    }

    return ConstDeclaration{ variables, literals };
}

Result<std::unique_ptr<Statement>> ASTBuilder::analyzeStatement(const ParseTree::Statement& node) {
    Result<std::unique_ptr<Expression>> result;

    switch (node.getType()) {
        case ParseTree::Statement::Type::ASSIGNMENT: {
            auto& assignment = node.asAssignmentExpression();

            result = analyzeExpression(assignment.getAdditiveExpression());
            if (result.failure()) {
                return result.error();
            }

            Result<symbol_id> target = symbolTable.useAsAssignmentTarget(assignment.getIdentifier());
            if (target.failure()) {
                return target.error(); // TODO implicit conversion?
            }

            std::unique_ptr<Statement> statement = std::make_unique<AssignmentStatement>(result.release(), Variable{ *target });
            return statement;
        }
        case ParseTree::Statement::Type::RETURN: {
            auto [returnKeyword, additiveExpression] = node.asReturnExpression();

            result = analyzeExpression(additiveExpression);
            if (result.failure()) {
                return result.error();
            }

            std::unique_ptr<Statement> statement = std::make_unique<ReturnStatement>(result.release());
            return statement;
        }
        case ParseTree::Statement::Type::NONE:
            return node.reference()
                .makeError(SourceCodeManagement::ErrorType::ERROR, "Encountered illegal parser tree state! Expected ASSIGNMENT or RETURN!");
    }

    return {};
}
Result<std::unique_ptr<Expression>> ASTBuilder::analyzeExpression(const ParseTree::AdditiveExpression& node) {
    Result<std::unique_ptr<Expression>> result;

    result = analyzeExpression(node.getExpression());
    if (!node.getOperand()) {
        return result;
    }

    if (result.failure()) {
        return result.error();
    }

    std::unique_ptr<Expression> multiplicativeExpression = result.release();
    auto [operatorTerminal, additiveExpression] = *node.getOperand();

    result = analyzeExpression(additiveExpression);
    if (result.failure()) {
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
            .makeError(SourceCodeManagement::ErrorType::ERROR, "Encountered illegal parse tree state! Expected PLUS or MINUS!");
    }
}
Result<std::unique_ptr<Expression>> ASTBuilder::analyzeExpression(const ParseTree::MultiplicativeExpression& node) {
    Result<std::unique_ptr<Expression>> result;

    result = analyzeExpression(node.getExpression());
    if (!node.getOperand()) {
        return result;
    }

    if (result.failure()) { // TODO implicit bool conversion?
        return result.error();
    }

    std::unique_ptr<Expression> unaryExpression = result.release();
    auto [operatorTerminal, multiplicativeExpression] = *node.getOperand();

    result = analyzeExpression(multiplicativeExpression);
    if (result.failure()) {
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
            .makeError(SourceCodeManagement::ErrorType::ERROR, "Encountered illegal parse tree state! Expected MULTIPLICATION or DIVISION!");
    }
}
Result<std::unique_ptr<Expression>> ASTBuilder::analyzeExpression(const ParseTree::UnaryExpression& node) {
    Result<std::unique_ptr<Expression>> result;

    result = analyzeExpression(node.getPrimaryExpression());
    if (!node.getUnaryOperator()) {
        return result;
    }

    if (result.failure()) {
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
            .makeError(SourceCodeManagement::ErrorType::ERROR, "Encountered illegal parse tree state! Expected PLUS or MINUS!");
    }
}
Result<std::unique_ptr<Expression>> ASTBuilder::analyzeExpression(const ParseTree::PrimaryExpression& node) {
    std::unique_ptr<Expression> expression;

    switch (node.getType()) {
        case ParseTree::PrimaryExpression::Type::IDENTIFIER: {
            Result<symbol_id> result = symbolTable.useIdentifier(node.asIdentifier());
            if (result.failure()) {
                return result.error();
            }

            expression = std::make_unique<Variable>(*result);
            return expression;
        }
        case ParseTree::PrimaryExpression::Type::LITERAL:
            expression = std::make_unique<Literal>(node.asLiteral().value());
            return expression;
        case ParseTree::PrimaryExpression::Type::ADDITIVE_EXPRESSION: {
            auto [openParenthesis, additiveExpression, closeParenthesis] = node.asBracketedExpression();
            return analyzeExpression(additiveExpression);
        }
        default:
            return node.reference()
                .makeError(SourceCodeManagement::ErrorType::ERROR, "Encountered illegal parse tree state! Expected IDENTIFIER, LITERAL or ADDITIVE_EXPRESSION!");
    }
}
//---------------------------------------------------------------------------
} // namespace pljit::ast
//---------------------------------------------------------------------------
