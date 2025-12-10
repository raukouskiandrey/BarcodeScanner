#include "BarcodeReader.h"
#include "country.h"
#include "manufacturer.h"
#include "product.h"
#include <iostream>
#include <fstream>

#include "ImageLoadException.h"
#include "DecodeException.h"
#include "FileException.h"

BarcodeReader::BarcodeReader()
    : smartDecoder(preprocessor, zbarDecoder) { // Правильная инициализация SmartDecoder
    // настройка ZBar и загрузка данных
}

BarcodeReader::~BarcodeReader() {
    // очистка данных
}

BarcodeResult BarcodeReader::decode(const cv::Mat& image) {
    return advancedDecode(image);
}

BarcodeResult BarcodeReader::decode(const std::string& filename) {
    cv::Mat image = cv::imread(filename);
    if (image.empty()) {
        throw ImageLoadException(filename); // выбрасываем исключение вместо возврата "Ошибка"
    }
    return advancedDecode(image);
}


BarcodeResult BarcodeReader::advancedDecode(const cv::Mat& image) {

    BarcodeResult result;
    result.type = "Неизвестно";

    if (image.empty()) {
        throw DecodeException("Пустое изображение для декодирования");
    }

    std::cout << "=== НАЧАЛО СКАНИРОВАНИЯ ===" << std::endl;
    std::cout << "Размер изображения: " << image.cols << "x" << image.rows << std::endl;

    cv::Mat frame = image.clone();

    // 1. ОБЫЧНЫЕ ШТРИХ-КОДЫ (OpenCV)
    auto polygons = opencvDetector.detectWithOpenCV(frame);
    std::cout << "OpenCV обнаружено полигонов: " << polygons.size() << std::endl;

    for (const auto& polygon : polygons) {
        if (polygon.size() != 4) continue;

        cv::Rect bbox = cv::boundingRect(polygon);
        cv::Mat roi = frame(bbox);
        std::string zbarResult = zbarDecoder.decodeWithZBar(roi);

        zbarResult = zbarDecoder.filterBarcodeResult(zbarResult);

        if (!zbarResult.empty()) {
            BarcodeResult parsedResult = zbarDecoder.parseZBarResult(zbarResult);
            if (parsedResult.type != "Неизвестно") {
                std::cout << "УСПЕХ: Распознан через OpenCV + ZBar" << std::endl;
                return createDetailedResult(parsedResult);
            }
        }
    }

    // 2. СЛОЖНЫЕ ШТРИХ-КОДЫ
    auto curved_regions = curvedDetector.detectCurvedBarcodesOptimized(frame);
    std::cout << "Обнаружено изогнутых регионов: " << curved_regions.size() << std::endl;

    for (const auto& rect : curved_regions) {
        std::string zbarResult = smartDecoder.smartDecodeWithUnwarp(frame, rect);

        if (!zbarResult.empty()) {
            BarcodeResult parsedResult = zbarDecoder.parseZBarResult(zbarResult);
            if (parsedResult.type != "Неизвестно") {
                std::cout << "УСПЕХ: Распознан через curved detection" << std::endl;
                return createDetailedResult(parsedResult);
            }
        }
    }

    // 3. ПРЯМОЙ СКАН ВСЕГО ИЗОБРАЖЕНИЯ ZBar
    std::cout << "Пытаемся прямой ZBar scan всего изображения..." << std::endl;
    std::string directResult = zbarDecoder.decodeWithZBar(frame);
    directResult = zbarDecoder.filterBarcodeResult(directResult);

    if (!directResult.empty()) {
        BarcodeResult parsedResult = zbarDecoder.parseZBarResult(directResult);
        if (parsedResult.type != "Неизвестно") {
            std::cout << "УСПЕХ: Распознан через прямой ZBar scan" << std::endl;
            return createDetailedResult(parsedResult);
        }
    }

    std::cout << "=== СКАНИРОВАНИЕ ЗАВЕРШЕНО, ШТРИХ-КОД НЕ РАСПОЗНАН ===" << std::endl;
    throw DecodeException("Штрих-код не распознан");
}

BarcodeResult BarcodeReader::createDetailedResult(const BarcodeResult& basicResult) {
    BarcodeResult detailedResult = basicResult;

    QString fullBarcode = QString::fromStdString(basicResult.digits);

    // --- Поиск товара ---
    try {
        QString productName = Product::findProductByBarcode(fullBarcode);
        if (!productName.isEmpty()) {
            detailedResult.productCode = productName.toStdString();
            std::cout << "Найден товар по штрих-коду: " << productName.toStdString() << std::endl;
        }
    }
    catch (const FileException& e) {
        std::cerr << e.what() << std::endl;
        detailedResult.productCode = "Ошибка чтения файла товаров";
    }

    if ((basicResult.type == "EAN-13" || basicResult.type == "UPCA") && basicResult.digits.length() >= 12) {
        std::string digitsToUse = basicResult.digits;

        if (basicResult.type == "UPCA" && digitsToUse.length() == 12) {
            digitsToUse = "0" + digitsToUse;
        }

        // --- Поиск страны ---
        if (digitsToUse.length() >= 3) {
            std::string country_code = digitsToUse.substr(0, 3);
            try {
                QString countryName = Country::findCountryByBarcode(QString::fromStdString(country_code));
                if (!countryName.isEmpty()) {
                    detailedResult.country = countryName.toStdString();
                } else {
                    detailedResult.country = "Неизвестная страна (" + country_code + ")";
                }
            }
            catch (const FileException& e) {
                std::cerr << e.what() << std::endl;
                detailedResult.country = "Ошибка чтения файла стран";
            }
        }

        // --- Поиск производителя ---
        if (digitsToUse.length() >= 7) {
            std::string manufacturer_code = digitsToUse.substr(3, 4);
            try {
                QString manufacturerName = Manufacturer::findManufacturerByCode(QString::fromStdString(manufacturer_code));
                if (!manufacturerName.isEmpty()) {
                    detailedResult.manufacturerCode = manufacturerName.toStdString();
                } else {
                    detailedResult.manufacturerCode = "Неизвестный производитель (" + manufacturer_code + ")";
                }
            }
            catch (const FileException& e) {
                std::cerr << e.what() << std::endl;
                detailedResult.manufacturerCode = "Ошибка чтения файла производителей";
            }

            if (detailedResult.productCode == "Н/Д" && digitsToUse.length() >= 7) {
                std::string product_code = digitsToUse.substr(7, 5);
                try {
                    QString additionalProductName = Product::findProductByBarcode(QString::fromStdString(product_code));
                    if (!additionalProductName.isEmpty()) {
                        detailedResult.productCode = additionalProductName.toStdString();
                    } else {
                        detailedResult.productCode = "Товар (" + product_code + ")";
                    }
                }
                catch (const FileException& e) {
                    std::cerr << e.what() << std::endl;
                    detailedResult.productCode = "Ошибка чтения файла товаров";
                }
            }
        }
    }
    else if (basicResult.type == "EAN-8" && basicResult.digits.length() == 8) {
        std::string country_code;
        try {
            if (basicResult.digits.length() >= 3) {
                country_code = basicResult.digits.substr(0, 3);
                QString countryName = Country::findCountryByBarcode(QString::fromStdString(country_code));
                if (!countryName.isEmpty()) {
                    detailedResult.country = countryName.toStdString();
                }
            }
            if (detailedResult.country.empty() && basicResult.digits.length() >= 2) {
                country_code = basicResult.digits.substr(0, 2);
                QString countryName = Country::findCountryByBarcode(QString::fromStdString(country_code));
                if (!countryName.isEmpty()) {
                    detailedResult.country = countryName.toStdString();
                } else {
                    detailedResult.country = "Неизвестная страна (" + country_code + ")";
                }
            }
        }
        catch (const FileException& e) {
            std::cerr << e.what() << std::endl;
            detailedResult.country = "Ошибка чтения файла стран";
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

void BarcodeReader::saveToFile(const BarcodeResult& result) {
    std::string filePath = "C:\\Users\\rauko\\Desktop\\Barcode_All.txt";
    std::ofstream file(filePath, std::ios::app);
    if (!file.is_open()) {
        throw FileException("Не удалось открыть файл для записи: " + filePath);
    }

    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::string timeStr = std::ctime(&time);
    if (!timeStr.empty() && timeStr.back() == '\n') {
        timeStr.pop_back();
    }

    file << "=== " << timeStr << " ===\n";
    file << "Тип: " << result.type << "\n";
    file << "Цифры: " << result.digits << "\n";
    file << "Страна: " << result.country << "\n";
    file << "Код производителя: " << result.manufacturerCode << "\n";
    file << "Код товара: " << result.productCode << "\n";
    file << "----------------------------------------\n";
    file.close();
}


