#include "resultdisplay.h"
#include "barcodescanner.h" // Включаем где определена структура
#include <iostream>

ResultDisplay::ResultDisplay()
{
}

std::string ResultDisplay::formatBarcodeResult(const BarcodeResult& result)
{
    std::string formatted;

    formatted += "\n=== РЕЗУЛЬТАТ СКАНИРОВАНИЯ ===\n";
    formatted += "Тип: " + result.type + "\n";
    formatted += "Полный код: " + result.digits + "\n";

    if (!result.country.empty() && result.country != "Неизвестно") {
        formatted += "Страна: " + result.country + "\n";
    }

    if (!result.manufacturerCode.empty() && result.manufacturerCode != "Н/Д" && result.manufacturerCode != "Нет") {
        formatted += "Код производителя: " + result.manufacturerCode + "\n";
    }

    if (!result.productCode.empty() && result.productCode != "Н/Д") {
        formatted += "Код товара: " + result.productCode + "\n";
    }

    return formatted;
}

void ResultDisplay::displayScanInfo(const std::string& info)
{
    std::cout << info << std::endl;
}

void ResultDisplay::showStatistics(const std::string& stats)
{
    std::cout << "=== СТАТИСТИКА ===" << std::endl;
    std::cout << stats << std::endl;
}
