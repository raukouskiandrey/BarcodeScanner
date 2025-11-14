#include "countrydataloader.h"
#include <fstream>
#include <iostream>

CountryDataLoader::CountryDataLoader()
{
}

CountryDataLoader::~CountryDataLoader()
{
    for (auto it = countries.begin(); it != countries.end(); ++it) {
        delete it.value();
    }
    countries.clear();
}

void CountryDataLoader::loadFromFile(const std::string& filePath)
{
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл кодов стран: " << filePath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        parseCountryLine(line);
    }

    file.close();
    std::cout << "Загружено стран: " << countries.size() << std::endl;
}

void CountryDataLoader::parseCountryLine(const std::string& line)
{
    size_t colon_pos = line.find(':');
    if (colon_pos != std::string::npos) {
        std::string codes = line.substr(0, colon_pos);
        std::string countryName = line.substr(colon_pos + 1);

        size_t dash_pos = codes.find('-');
        if (dash_pos != std::string::npos) {
            std::string start_str = codes.substr(0, dash_pos);
            std::string end_str = codes.substr(dash_pos + 1);

            try {
                int start = std::stoi(start_str);
                int end = std::stoi(end_str);

                for (int i = start; i <= end; i++) {
                    std::string code = std::to_string(i);
                    Country* country = new Country(QString::fromStdString(code),
                                                   QString::fromStdString(countryName));
                    countries[QString::fromStdString(code)] = country;
                }
            } catch (...) {
                std::cerr << "Ошибка парсинга диапазона: " << codes << std::endl;
            }
        } else {
            Country* country = new Country(QString::fromStdString(codes),
                                           QString::fromStdString(countryName));
            countries[QString::fromStdString(codes)] = country;
        }
    }
}

Country* CountryDataLoader::getCountryByCode(const QString& code)
{
    if (countries.contains(code)) {
        return countries[code];
    }

    for (int len = code.length() - 1; len >= 2; len--) {
        QString partialCode = code.left(len);
        if (countries.contains(partialCode)) {
            return countries[partialCode];
        }
    }

    return nullptr;
}
