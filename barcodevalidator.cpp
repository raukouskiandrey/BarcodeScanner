#include "barcodevalidator.h"
#include <iostream>

BarcodeValidator::BarcodeValidator()
{
}

bool BarcodeValidator::validateChecksum(const std::string& ean)
{
    return true;
}

std::string BarcodeValidator::filterBarcodeResult(const std::string& result)
{
    if (result.empty()) return "";

    // Разрешаем все типы штрих-кодов, которые может обнаружить ZBar
    std::cout << "ZBar raw result: " << result << std::endl;
    return result;
}
