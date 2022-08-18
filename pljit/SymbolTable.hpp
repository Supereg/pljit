//
// Created by Andreas Bauer on 03.08.22.
//

#ifndef PLJIT_SYMBOLTABLE_HPP
#define PLJIT_SYMBOLTABLE_HPP

#include "./symbol_id.hpp"
#include "pljit/code/SourceCodeManagement.hpp"
#include "pljit/parse/ParseTree.hpp"
#include "pljit/util/Result.hpp"
#include <unordered_map>
#include <vector>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
class SymbolTable {
    public:

    class Symbol {
        friend class SymbolTable;

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

        private:
        void markInitialized();
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

    std::size_t size() const;

    std::optional<Symbol> retrieveSymbol(symbol_id symbolId);
    std::optional<Symbol> retrieveSymbol(std::string_view identifier_name);
    Symbol& operator[](symbol_id symbolId);

    Result<symbol_id> declareIdentifier(const parse::Identifier& identifier, SymbolType symbolType);
    Result<symbol_id> useIdentifier(const parse::Identifier& identifier);
    Result<symbol_id> useAsAssignmentTarget(const parse::Identifier& identifier);
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_SYMBOLTABLE_HPP
