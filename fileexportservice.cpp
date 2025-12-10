#include "fileexportservice.h"
#include "barcodescanner.h" // Включаем где определена структура
#include <fstream>
#include <chrono>
#include <ctime>
#include <map>

FileExportService::FileExportService()
{
}

void FileExportService::saveToMainFile(const BarcodeResult& result)
{
    std::string filePath = "C:\\Users\\rauko\\Desktop\\Barcode_All.txt";

    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::string timeStr = std::ctime(&time);
    if (!timeStr.empty() && timeStr[timeStr.length()-1] == '\n') {
        timeStr.erase(timeStr.length()-1);
    }

    std::string content = "=== " + timeStr + " ===\n" +
                          "Тип: " + result.type + "\n" +
                          "Цифры: " + result.digits + "\n" +
                          "Страна: " + result.country + "\n" +
                          "Код производителя: " + result.manufacturerCode + "\n" +
                          "Код товара: " + result.productCode + "\n" +
                          "----------------------------------------\n";

    appendToFile(filePath, content);
}

void FileExportService::saveToCountryFile(const BarcodeResult& result)
{
    if (result.country.empty() || result.country == "Неизвестно") return;

    std::string filePath = "C:\\Users\\rauko\\Desktop\\Barcode_Countries.txt";

    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::string timeStr = std::ctime(&time);
    if (!timeStr.empty() && timeStr[timeStr.length()-1] == '\n') {
        timeStr.erase(timeStr.length()-1);
    }

    std::string content = "[" + timeStr + "] " +
                          result.type + " " + result.digits +
                          " -> " + result.country + "\n";

    appendToFile(filePath, content);
}

void FileExportService::saveToManufacturerFile(const BarcodeResult& result)
{
    if (result.manufacturerCode.empty() || result.manufacturerCode == "Н/Д" || result.manufacturerCode == "Нет") return;

    std::string filePath = "C:\\Users\\rauko\\Desktop\\Barcode_Manufacturers.txt";

    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::string timeStr = std::ctime(&time);
    if (!timeStr.empty() && timeStr[timeStr.length()-1] == '\n') {
        timeStr.erase(timeStr.length()-1);
    }

    std::string content = "[" + timeStr + "] " +
                          result.type + " " + result.digits +
                          " -> Код производителя: " + result.manufacturerCode +
                          " (Страна: " + result.country + ")\n";

    appendToFile(filePath, content);
}

void FileExportService::saveToProductFile(const BarcodeResult& result)
{
    if (result.productCode.empty() || result.productCode == "Н/Д") return;

    std::string filePath = "C:\\Users\\rauko\\Desktop\\Barcode_Products.txt";

    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::string timeStr = std::ctime(&time);
    if (!timeStr.empty() && timeStr[timeStr.length()-1] == '\n') {
        timeStr.erase(timeStr.length()-1);
    }

    std::string content = "[" + timeStr + "] " +
                          result.type + " " + result.digits +
                          " -> Код товара: " + result.productCode +
                          " (Производитель: " + result.manufacturerCode + ")\n";

    appendToFile(filePath, content);
}

void FileExportService::saveToStatisticsFile(const BarcodeResult& result)
{
    std::string filePath = "C:\\Users\\rauko\\Desktop\\Barcode_Statistics.txt";

    std::map<std::string, int> countryStats;
    std::map<std::string, int> typeStats;

    std::ifstream inFile(filePath);
    std::string line;

    while (std::getline(inFile, line)) {
        if (line.find("Страна:") != std::string::npos) {
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                std::string country = line.substr(pos + 2);
                country = country.substr(0, country.find(" -"));
                int count = std::stoi(line.substr(line.rfind(" ") + 1));
                countryStats[country] = count;
            }
        }
        else if (line.find("Тип:") != std::string::npos) {
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                std::string type = line.substr(pos + 2);
                type = type.substr(0, type.find(" -"));
                int count = std::stoi(line.substr(line.rfind(" ") + 1));
                typeStats[type] = count;
            }
        }
    }
    inFile.close();

    countryStats[result.country]++;
    typeStats[result.type]++;

    std::ofstream outFile(filePath);
    outFile << "=== СТАТИСТИКА СКАНИРОВАНИЙ ===" << std::endl;
    outFile << "Обновлено: ";

    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::string timeStr = std::ctime(&time);
    if (!timeStr.empty() && timeStr[timeStr.length()-1] == '\n') {
        timeStr.erase(timeStr.length()-1);
    }
    outFile << timeStr << std::endl << std::endl;

    outFile << "ПО СТРАНАМ:" << std::endl;
    for (const auto& [country, count] : countryStats) {
        outFile << "Страна: " << country << " - " << count << " шт." << std::endl;
    }

    outFile << std::endl << "ПО ТИПАМ:" << std::endl;
    for (const auto& [type, count] : typeStats) {
        outFile << "Тип: " + type + " - " + std::to_string(count) + " шт.\n";
    }

    outFile.close();
}

void FileExportService::saveToSimpleFile(const std::string& barcodeData)
{
    std::string filePath = "C:\\Users\\rauko\\Desktop\\Barcode_Simple.txt";

    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::string timeStr = std::ctime(&time);
    if (!timeStr.empty() && timeStr[timeStr.length()-1] == '\n') {
        timeStr.erase(timeStr.length()-1);
    }

    std::string content = "[" + timeStr + "] Штрих-код: " + barcodeData + "\n";
    appendToFile(filePath, content);
}

void FileExportService::appendToFile(const std::string& filePath, const std::string& content)
{
    std::ofstream file(filePath, std::ios::app);
    if (file.is_open()) {
        file << content;
        file.close();
    }
}
