//
// Created by Andreas Bauer on 03.08.22.
//

#ifndef PLJIT_SYMBOLTABLE_HPP
#define PLJIT_SYMBOLTABLE_HPP

#include "../code/SourceCodeManagement.hpp"
#include "../parse/ParseTree.hpp"
#include "../util/Result.hpp"
#include <unordered_map>
#include <vector>

//---------------------------------------------------------------------------
namespace pljit::ast {
//---------------------------------------------------------------------------
using symbol_id = std::size_t;
//---------------------------------------------------------------------------
class SymbolTable {
    public:

    class Symbol {
        private:
        code::SourceCodeReference src_reference;
        symbol_id symbolId;
        bool constant;
        bool initialized;

        public:
        Symbol(code::SourceCodeReference src_reference, symbol_id symbolId, bool constant, bool initialized);

        const code::SourceCodeReference& reference() const;

        symbol_id id() const;
        bool isConstant() const;
        bool isInitialized() const;
    };

    enum class SymbolType {
        PARAM,
        VAR,
        CONST,
    };

    private:
    std::vector<Symbol> symbols; // symbols indexed by their id
    std::unordered_map<std::string_view, symbol_id> symbolLookup; // symbol ids, indexed by their name!

    public:
    SymbolTable();

    // TODO get a reference to mark something initialized now! (or even mark it constant?)
    std::optional<Symbol> retrieveSymbol(symbol_id symbolId);
    std::optional<Symbol> retrieveSymbol(std::string_view identifier_name);

    Result<symbol_id> declareIdentifier(const parse::ParseTree::Identifier& identifier, SymbolType symbolType);
    Result<symbol_id> useIdentifier(const parse::ParseTree::Identifier& identifier);
    Result<symbol_id> useAsAssignmentTarget(const parse::ParseTree::Identifier& identifier);

};
//---------------------------------------------------------------------------
} // namespace pljit::ast
//---------------------------------------------------------------------------

#endif //PLJIT_SYMBOLTABLE_HPP
