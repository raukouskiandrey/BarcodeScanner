#pragma once
#include <string>

class BarcodeResult {
public:
    std::string type;
    std::string digits;
    std::string fullResult;
    std::string country;
    std::string manufacturerCode;
    std::string productCode;
};
