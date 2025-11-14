#include "productdataloader.h"
#include <fstream>
#include <iostream>

ProductDataLoader::ProductDataLoader()
{
}

ProductDataLoader::~ProductDataLoader()
{
    for (auto it = products.begin(); it != products.end(); ++it) {
        delete it.value();
    }
    products.clear();
    productsByCode.clear();
}

void ProductDataLoader::loadFromFile(const std::string& filePath)
{
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл товаров: " << filePath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        parseProductLine(line);
    }

    file.close();
    std::cout << "Загружено товаров: " << products.size() << std::endl;
}

void ProductDataLoader::parseProductLine(const std::string& line)
{
    size_t first_pipe = line.find('|');
    if (first_pipe != std::string::npos) {
        std::string barcode = line.substr(0, first_pipe);
        barcode.erase(0, barcode.find_first_not_of(" \t"));
        barcode.erase(barcode.find_last_not_of(" \t") + 1);

        std::string remaining = line.substr(first_pipe + 1);

        size_t second_pipe = remaining.find('|');
        if (second_pipe != std::string::npos) {
            std::string manufacturer = remaining.substr(0, second_pipe);
            std::string productName = remaining.substr(second_pipe + 1);

            manufacturer.erase(0, manufacturer.find_first_not_of(" \t"));
            manufacturer.erase(manufacturer.find_last_not_of(" \t") + 1);

            productName.erase(0, productName.find_first_not_of(" \t"));
            productName.erase(productName.find_last_not_of(" \t") + 1);

            std::string productCode = "00000";
            if (barcode.length() >= 5) {
                productCode = barcode.substr(barcode.length() - 5);
            }

            Product* product = new Product(
                QString::fromStdString(productCode),
                QString::fromStdString(productName),
                QString::fromStdString(barcode)
                );

            products[QString::fromStdString(barcode)] = product;
            productsByCode[QString::fromStdString(productCode)] = product;
        }
    }
}

Product* ProductDataLoader::getProductByBarcode(const QString& barcode)
{
    return products.contains(barcode) ? products[barcode] : nullptr;
}

Product* ProductDataLoader::getProductByCode(const QString& code)
{
    return productsByCode.contains(code) ? productsByCode[code] : nullptr;
}
