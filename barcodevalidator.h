#ifndef BARCODEVALIDATOR_H
#define BARCODEVALIDATOR_H

#include <string>

class BarcodeValidator
{
public:
    BarcodeValidator();

    static bool validateChecksum(const std::string& ean);
    std::string filterBarcodeResult(const std::string& result);
};

#endif // BARCODEVALIDATOR_H
