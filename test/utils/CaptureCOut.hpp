//
// Created by Andreas Bauer on 14.07.22.
//

#ifndef PLJIT_CAPTURECOUT_HPP
#define PLJIT_CAPTURECOUT_HPP

#include <iostream>
#include <sstream>

//---------------------------------------------------------------------------
namespace {
//---------------------------------------------------------------------------
class CaptureCOut { // taken from the AST exercise
    private:
    std::streambuf* sbuf;

    public:
    std::stringstream stream;

    CaptureCOut() : sbuf(std::cout.rdbuf()) {
        std::cout.rdbuf(stream.rdbuf());
    }

    ~CaptureCOut() {
        stopCapture();
    }

    void stopCapture() const {
        std::cout.rdbuf(sbuf);
    }

    auto str() const {
        return stream.str();
    }
};
//---------------------------------------------------------------------------
} // namespace
//---------------------------------------------------------------------------

#endif //PLJIT_CAPTURECOUT_HPP
