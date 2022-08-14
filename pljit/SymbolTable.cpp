//
// Created by Andreas Bauer on 03.08.22.
//

#include "SymbolTable.hpp"
#include "pljit/code/SourceCodeManagement.hpp"
#include "pljit/parse/ParseTree.hpp"
#include <cassert>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
SymbolTable::SymbolTable() : symbols(), symbolLookup() {}

std::size_t SymbolTable::size() const {
    return symbols.size();
}

std::optional<SymbolTable::Symbol> SymbolTable::retrieveSymbol(symbol_id symbolId) {
    assert(symbolId > 0 && "Encountered illegal symbol id!");
    std::size_t index = symbolId - 1;

    if (index >= symbols.size()) {
        return {};
    }

    return { symbols[index] };
}

std::optional<SymbolTable::Symbol> SymbolTable::retrieveSymbol(std::string_view identifier_name) {
    if (!symbolLookup.contains(identifier_name)) {
        return {};
    }

    return { retrieveSymbol(symbolLookup[identifier_name]) };
}

SymbolTable::Symbol& SymbolTable::operator[](symbol_id symbolId) {
    assert(symbolId > 0 && "Encountered illegal symbol id!");
    std::size_t index = symbolId - 1;
    return symbols[index];
}

Result<symbol_id> SymbolTable::declareIdentifier(const parse::ParseTree::Identifier& identifier, SymbolType symbolType) {
    std::optional<Symbol> existingSymbol = retrieveSymbol(identifier.value());
    if (existingSymbol) {
        return identifier.reference()
            .makeError(code::SourceCodeManagement::ErrorType::ERROR, "Redefinition of identifier!")
            .withCause(
                existingSymbol->reference()
                    .makeError(code::SourceCodeManagement::ErrorType::NOTE, "Original declaration here"));
    }

    symbol_id id = symbols.size() + 1; // symbol id of 0 is invalid!

    bool isConstant = symbolType == SymbolType::CONST;
    bool isInitialized = symbolType == SymbolType::CONST || symbolType == SymbolType::PARAM;
    symbols.emplace_back(identifier.reference(), id, isConstant, isInitialized);

    symbolLookup.emplace(identifier.value(), id);

    return id;
}

Result<symbol_id> SymbolTable::useIdentifier(const parse::ParseTree::Identifier& identifier) {
    std::optional<Symbol> existingSymbol = retrieveSymbol(identifier.value());
    if (!existingSymbol) {
        return identifier.reference()
            .makeError(code::SourceCodeManagement::ErrorType::ERROR, "Using undeclared identifier!");
    }

    if (!existingSymbol->isInitialized()) {
        return identifier.reference()
            .makeError(code::SourceCodeManagement::ErrorType::ERROR, "Tried to use uninitialized variable!");
    }

    return existingSymbol->id();
}

Result<symbol_id> SymbolTable::useAsAssignmentTarget(const parse::ParseTree::Identifier& identifier) {
    std::optional<Symbol> existingSymbol = retrieveSymbol(identifier.value());
    if (!existingSymbol) {
        return identifier.reference()
            .makeError(code::SourceCodeManagement::ErrorType::ERROR, "Using undeclared identifier!");
    }

    if (existingSymbol->isConstant()) {
        return identifier.reference()
            .makeError(code::SourceCodeManagement::ErrorType::ERROR, "Can't assign to constant!");
    }

    operator[](existingSymbol->id()).markInitialized();

    return existingSymbol->id();
}
//---------------------------------------------------------------------------
SymbolTable::Symbol::Symbol(code::SourceCodeReference src_reference, symbol_id symbolId, bool constant, bool initialized)
    : src_reference(src_reference), symbolId(symbolId), constant(constant), initialized(initialized) {}

const code::SourceCodeReference& SymbolTable::Symbol::reference() const {
    return src_reference;
}

symbol_id SymbolTable::Symbol::id() const {
    return symbolId;
}

bool SymbolTable::Symbol::isConstant() const {
    return constant;
}

bool SymbolTable::Symbol::isInitialized() const {
    return initialized;
}

void SymbolTable::Symbol::markInitialized() {
    initialized = true;
}
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------