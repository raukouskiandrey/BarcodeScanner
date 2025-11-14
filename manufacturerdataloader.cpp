#include "manufacturerdataloader.h"
#include <fstream>
#include <iostream>

ManufacturerDataLoader::ManufacturerDataLoader()
{
}

ManufacturerDataLoader::~ManufacturerDataLoader()
{
    for (auto it = manufacturers.begin(); it != manufacturers.end(); ++it) {
        delete it.value();
    }
    manufacturers.clear();
}

void ManufacturerDataLoader::loadFromFile(const std::string& filePath)
{
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл производителей: " << filePath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        parseManufacturerLine(line);
    }

    file.close();
    std::cout << "Загружено производителей: " << manufacturers.size() << std::endl;
}

void ManufacturerDataLoader::parseManufacturerLine(const std::string& line)
{
    size_t first_colon = line.find(':');
    if (first_colon != std::string::npos) {
        std::string manufacturerCode = line.substr(0, first_colon);
        std::string remaining = line.substr(first_colon + 1);

        size_t second_colon = remaining.find(':');
        if (second_colon != std::string::npos) {
            std::string manufacturerName = remaining.substr(0, second_colon);
            std::string countryCode = remaining.substr(second_colon + 1);

            Manufacturer* manufacturer = new Manufacturer(
                QString::fromStdString(manufacturerCode),
                QString::fromStdString(manufacturerName),
                QString::fromStdString(countryCode)
                );
            manufacturers[QString::fromStdString(manufacturerCode)] = manufacturer;
        }
    }
}

Manufacturer* ManufacturerDataLoader::getManufacturerByCode(const QString& code)
{
    return manufacturers.contains(code) ? manufacturers[code] : nullptr;
}
