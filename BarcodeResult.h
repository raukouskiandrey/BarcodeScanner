// BarcodeResult.h
#pragma once
#include <string>

struct BarcodeResult {
    std::string type;
    std::string digits;
    std::string fullResult;
    std::string country;
    std::string manufacturerCode;
    std::string productCode;
};
