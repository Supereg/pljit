//
// Created by Andreas Bauer on 21.08.22.
//

#include "./CaptureCOut.hpp"

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
CaptureCOut::CaptureCOut() : sbuf(std::cout.rdbuf()) {
    std::cout.rdbuf(stream.rdbuf());
}
CaptureCOut::~CaptureCOut() {
    stopCapture();
}

void CaptureCOut::stopCapture() const {
    std::cout.rdbuf(sbuf);
}

std::string CaptureCOut::str() const {
    return stream.str();
}
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------
