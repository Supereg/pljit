//
// Created by Andreas Bauer on 14.07.22.
//

#ifndef PLJIT_CAPTURECOUT_HPP
#define PLJIT_CAPTURECOUT_HPP

#include <iostream>
#include <sstream>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
class CaptureCOut { // taken from the AST exercise
    private:
    std::streambuf* sbuf;

    public:
    std::stringstream stream;

    CaptureCOut();
    ~CaptureCOut();

    void stopCapture() const;

    std::string str() const;
};
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------

#endif //PLJIT_CAPTURECOUT_HPP
