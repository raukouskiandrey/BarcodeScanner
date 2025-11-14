#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#define _CRT_SECURE_NO_WARNINGS

#include "barcodescanner.h"
#include <string>
#include <fstream>
#include <chrono>
#include <ctime>
#include <map>
#include <iostream>
#include <algorithm>
#include <sstream>

BarcodeReader::BarcodeReader()
{
    std::cout << "=== ЗАГРУЗКА ДАННЫХ BARCODE READER ===" << std::endl;
    loadData();
    std::cout << "=== ДАННЫЕ ЗАГРУЖЕНЫ ===" << std::endl;
}

BarcodeReader::~BarcodeReader()
{
    cleanupData();
}

void BarcodeReader::loadData()
{
    countryLoader.loadFromFile();
    manufacturerLoader.loadFromFile();
    productLoader.loadFromFile();
}

void BarcodeReader::cleanupData()
{
    // Data loaders now handle their own cleanup
    std::cout << "BarcodeReader data cleaned up" << std::endl;
}

BarcodeReader::BarcodeResult BarcodeReader::createDetailedResult(const BarcodeResult& basicResult)
{
    return createDetailedResultWithClasses(basicResult);
}

BarcodeReader::BarcodeResult BarcodeReader::createDetailedResultWithClasses(const BarcodeResult& basicResult)
{
    BarcodeResult detailedResult = basicResult;

    QString fullBarcode = QString::fromStdString(basicResult.digits);
    Product* product = productLoader.getProductByBarcode(fullBarcode);
    if (product && product->isValid()) {
        detailedResult.productCode = product->getProductName().toStdString();
        std::cout << "Найден товар по штрих-коду: " << product->getFullInfo().toStdString() << std::endl;
    }

    if ((basicResult.type == "EAN-13" || basicResult.type == "UPCA") && basicResult.digits.length() >= 12) {
        std::string digitsToUse = basicResult.digits;

        if (basicResult.type == "UPCA" && digitsToUse.length() == 12) {
            digitsToUse = "0" + digitsToUse;
        }

        if (digitsToUse.length() >= 3) {
            std::string country_code = digitsToUse.substr(0, 3);
            Country* country = countryLoader.getCountryByCode(QString::fromStdString(country_code));

            if (country && country->isValid()) {
                detailedResult.country = country->getFullInfo().toStdString();
            } else {
                detailedResult.country = "Неизвестная страна (" + country_code + ")";
            }
        }

        if (digitsToUse.length() >= 7) {
            std::string manufacturer_code = digitsToUse.substr(3, 4);
            Manufacturer* manufacturer = manufacturerLoader.getManufacturerByCode(QString::fromStdString(manufacturer_code));

            if (manufacturer && manufacturer->isValid()) {
                detailedResult.manufacturerCode = manufacturer->getFullInfo().toStdString();
            } else {
                detailedResult.manufacturerCode = "Неизвестный производитель (" + manufacturer_code + ")";
            }

            if (!product && digitsToUse.length() >= 7) {
                std::string product_code = digitsToUse.substr(7, 5);
                product = productLoader.getProductByCode(QString::fromStdString(product_code));
                if (product && product->isValid()) {
                    detailedResult.productCode = product->getProductName().toStdString();
                } else {
                    detailedResult.productCode = "Товар (" + product_code + ")";
                }
            }
        }
    }
    else if (basicResult.type == "EAN-8" && basicResult.digits.length() == 8) {
        if (basicResult.digits.length() >= 2) {
            std::string country_code;
            Country* country = nullptr;

            if (basicResult.digits.length() >= 3) {
                country_code = basicResult.digits.substr(0, 3);
                country = countryLoader.getCountryByCode(QString::fromStdString(country_code));
            }

            if (!country && basicResult.digits.length() >= 2) {
                country_code = basicResult.digits.substr(0, 2);
                country = countryLoader.getCountryByCode(QString::fromStdString(country_code));
            }

            if (country && country->isValid()) {
                detailedResult.country = country->getFullInfo().toStdString();
            } else if (!country_code.empty()) {
                detailedResult.country = "Неизвестная страна (" + country_code + ")";
            }
        }

        detailedResult.manufacturerCode = "Нет";
        if (basicResult.digits.length() >= 5) {
            std::string product_code = basicResult.digits.substr(2, 5);
            detailedResult.productCode = "Товар (" + product_code + ")";
        } else if (basicResult.digits.length() >= 3) {
            std::string product_code = basicResult.digits.substr(2);
            detailedResult.productCode = "Товар (" + product_code + ")";
        }
    }
    else if (basicResult.type == "UPCE" && basicResult.digits.length() == 8) {
        detailedResult.country = "Специальный формат UPC-E";
        detailedResult.manufacturerCode = "Н/Д";
        detailedResult.productCode = "Н/Д";
    }
    else if (basicResult.type == "CODE128" || basicResult.type == "CODE39") {
        detailedResult.country = "Не применимо";
        detailedResult.manufacturerCode = "Н/Д";
        detailedResult.productCode = "Н/Д";
    }

    if (detailedResult.country.empty()) detailedResult.country = "Неизвестно";
    if (detailedResult.manufacturerCode.empty()) detailedResult.manufacturerCode = "Н/Д";
    if (detailedResult.productCode.empty()) detailedResult.productCode = "Н/Д";

    std::cout << "Final detailed result - Country: " << detailedResult.country
              << ", Manufacturer: " << detailedResult.manufacturerCode
              << ", Product: " << detailedResult.productCode << std::endl;

    return detailedResult;
}

BarcodeReader::BarcodeResult BarcodeReader::decode(const cv::Mat& image)
{
    return advancedDecode(image);
}

BarcodeReader::BarcodeResult BarcodeReader::decode(const std::string& filename)
{
    cv::Mat image = cv::imread(filename);
    if (image.empty()) {
        BarcodeResult result;
        result.type = "Ошибка";
        result.digits = "Не удалось загрузить изображение";
        return result;
    }
    return advancedDecode(image);
}

void BarcodeReader::saveToFile(const BarcodeResult& result)
{
    FileExportService::saveToMainFile(result);
    FileExportService::saveToCountryFile(result);
    FileExportService::saveToManufacturerFile(result);
    FileExportService::saveToProductFile(result);
    FileExportService::saveToStatisticsFile(result);
}

void BarcodeReader::saveToFile(const std::string& barcodeData)
{
    FileExportService::saveToSimpleFile(barcodeData);
}

BarcodeReader::BarcodeResult BarcodeReader::advancedDecode(const cv::Mat& image)
{
    BarcodeResult result;
    result.type = "Неизвестно";

    if (image.empty()) {
        result.type = "Ошибка";
        result.digits = "Пустое изображение";
        return result;
    }

    std::cout << "=== НАЧАЛО СКАНИРОВАНИЯ ===" << std::endl;
    std::cout << "Размер изображения: " << image.cols << "x" << image.rows << std::endl;

    cv::Mat frame = image.clone();

    // 1. ОБЫЧНЫЕ ШТРИХ-КОДЫ (OpenCV)
    auto polygons = detector.detectWithOpenCV(frame);
    std::cout << "OpenCV обнаружено полигонов: " << polygons.size() << std::endl;

    for (const auto& polygon : polygons) {
        if (polygon.size() != 4) continue;

        cv::Rect bbox = cv::boundingRect(polygon);
        cv::Mat roi = frame(bbox);
        std::string zbarResult = decoder.decodeWithZBar(roi);

        zbarResult = validator.filterBarcodeResult(zbarResult);

        if (!zbarResult.empty()) {
            BarcodeResult parsedResult = parseZBarResult(zbarResult);
            if (parsedResult.type != "Неизвестно") {
                std::cout << "УСПЕХ: Распознан через OpenCV + ZBar" << std::endl;
                return createDetailedResult(parsedResult);
            }
        }
    }

    // 2. СЛОЖНЫЕ ШТРИХ-КОДЫ
    std::vector<cv::Rect> curved_regions = detector.detectCurvedBarcodesOptimized(frame);
    std::cout << "Обнаружено изогнутых регионов: " << curved_regions.size() << std::endl;

    for (const auto& rect : curved_regions) {
        std::string zbarResult = decoder.smartDecodeWithUnwarp(frame, rect);

        if (!zbarResult.empty()) {
            BarcodeResult parsedResult = parseZBarResult(zbarResult);
            if (parsedResult.type != "Неизвестно") {
                std::cout << "УСПЕХ: Распознан через curved detection" << std::endl;
                return createDetailedResult(parsedResult);
            }
        }
    }

    // 3. ПРЯМОЙ СКАН ВСЕГО ИЗОБРАЖЕНИЯ ZBar
    std::cout << "Пытаемся прямой ZBar scan всего изображения..." << std::endl;
    std::string directResult = decoder.decodeWithZBar(frame);
    directResult = validator.filterBarcodeResult(directResult);

    if (!directResult.empty()) {
        BarcodeResult parsedResult = parseZBarResult(directResult);
        if (parsedResult.type != "Неизвестно") {
            std::cout << "УСПЕХ: Распознан через прямой ZBar scan" << std::endl;
            return createDetailedResult(parsedResult);
        }
    }

    std::cout << "=== СКАНИРОВАНИЕ ЗАВЕРШЕНО, ШТРИХ-КОД НЕ РАСПОЗНАН ===" << std::endl;
    return result;
}

BarcodeReader::BarcodeResult BarcodeReader::parseZBarResult(const std::string& zbarResult)
{
    BarcodeResult result;
    result.type = "Неизвестно";

    if (zbarResult.empty()) {
        return result;
    }

    // Парсим результат ZBar
    size_t colon_pos = zbarResult.find(":");
    if (colon_pos != std::string::npos) {
        std::string type = zbarResult.substr(0, colon_pos);
        std::string digits = zbarResult.substr(colon_pos + 2); // +2 чтобы пропустить ": "

        result.type = type;
        result.digits = digits;
        result.fullResult = zbarResult;
    } else {
        // Если формат нестандартный, пытаемся извлечь данные
        result.type = "Unknown Format";
        result.digits = zbarResult;
        result.fullResult = zbarResult;
    }

    return result;
}
