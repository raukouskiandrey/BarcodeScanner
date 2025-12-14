#include "BarcodeReader.h"
#include "Country.h"
#include "Manufacturer.h"
#include "Product.h"
#include <iostream>
#include <fstream>

#include "ImageLoadException.h"
#include "DecodeException.h"
#include "FileException.h"

BarcodeReader::BarcodeReader()
    : smartDecoder(preprocessor, zbarDecoder) { // Правильная инициализация SmartDecoder
    // настройка ZBar и загрузка данных
}

BarcodeReader::~BarcodeReader() = default;

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

std::string BarcodeReader::findProduct(const QString& barcode) {
    try {
        QString productName = Product::findProductByBarcode(barcode);
        if (!productName.isEmpty()) {
            std::cout << "Найден товар по штрих-коду: " << productName.toStdString() << std::endl;
            return productName.toStdString();
        }
    } catch (const FileException& e) {
        std::cerr << e.what() << std::endl;
        return "Ошибка чтения файла товаров";
    }
    return "Н/Д";
}

std::string BarcodeReader::findCountry(std::string_view digits) {
    if (digits.length() < 3) return "Неизвестно";
    auto country_code = std::string(digits.substr(0, 3));
    try {
        QString countryName = Country::findCountryByBarcode(QString::fromStdString(country_code));
        return countryName.isEmpty() ? "Неизвестная страна (" + country_code + ")" : countryName.toStdString();
    } catch (const FileException& e) {
        std::cerr << e.what() << std::endl;
        return "Ошибка чтения файла стран";
    }
}

std::string BarcodeReader::findManufacturer(std::string_view digits) {
    if (digits.length() < 7) return "Н/Д";
    auto manufacturer_code = std::string(digits.substr(3, 4));
    try {
        QString manufacturerName = Manufacturer::findManufacturerByCode(QString::fromStdString(manufacturer_code));
        return manufacturerName.isEmpty() ? "Неизвестный производитель (" + manufacturer_code + ")" : manufacturerName.toStdString();
    } catch (const FileException& e) {
        std::cerr << e.what() << std::endl;
        return "Ошибка чтения файла производителей";
    }
}

std::string BarcodeReader::findAdditionalProduct(std::string_view digits) {
    if (digits.length() < 7) return "Н/Д";
    auto product_code = std::string(digits.substr(7, 5));
    try {
        QString productName = Product::findProductByBarcode(QString::fromStdString(product_code));
        return productName.isEmpty() ? "Товар (" + product_code + ")" : productName.toStdString();
    } catch (const FileException& e) {
        std::cerr << e.what() << std::endl;
        return "Ошибка чтения файла товаров";
    }
}

// Проверка типа EAN-13 или UPC-A
bool BarcodeReader::isEAN13orUPCA(const BarcodeResult& result) {
    return (result.type == "EAN-13" || result.type == "UPCA") && result.digits.length() >= 12;
}

// Проверка типа EAN-8
bool BarcodeReader::isEAN8(const BarcodeResult& result) {
    return (result.type == "EAN-8" && result.digits.length() == 8);
}

// Нормализация цифр для UPC-A (добавляем ведущий ноль)
std::string BarcodeReader::normalizeDigits(const BarcodeResult& result) {
    std::string digitsToUse = result.digits;
    if (result.type == "UPCA" && digitsToUse.length() == 12) {
        digitsToUse = "0" + digitsToUse;
    }
    return digitsToUse;
}

// Поиск страны для EAN-8
std::string BarcodeReader::findCountryForEAN8(std::string_view digits) {
    try {
        if (digits.length() >= 3) {
            auto code = std::string(digits.substr(0, 3));
            QString countryName = Country::findCountryByBarcode(QString::fromStdString(code));
            if (!countryName.isEmpty()) return countryName.toStdString();
        }
        if (digits.length() >= 2) {
            auto code = std::string(digits.substr(0, 2));
            if (QString countryName = Country::findCountryByBarcode(QString::fromStdString(code)); !countryName.isEmpty()) return countryName.toStdString();
            return "Неизвестная страна (" + code + ")";
        }
    } catch (const FileException& e) {
        std::cerr << e.what() << std::endl;
        return "Ошибка чтения файла стран";
    }
    return "Неизвестно";
}

// Поиск товара для EAN-8
std::string BarcodeReader::findProductForEAN8(std::string_view digits) {
    if (digits.length() >= 5) {
        auto product_code = std::string(digits.substr(2, 5));
        return "Товар (" + product_code + ")";
    } else if (digits.length() >= 3) {
        auto product_code = std::string(digits.substr(2));
        return "Товар (" + product_code + ")";
    }
    return "Н/Д";
}

BarcodeResult BarcodeReader::createDetailedResult(const BarcodeResult& basicResult) {
    BarcodeResult detailedResult = basicResult;
    QString fullBarcode = QString::fromStdString(basicResult.digits);

    // --- Поиск товара ---
    detailedResult.productCode = findProduct(fullBarcode);

    if (isEAN13orUPCA(basicResult)) {
        std::string digitsToUse = normalizeDigits(basicResult);
        detailedResult.country = findCountry(digitsToUse);
        detailedResult.manufacturerCode = findManufacturer(digitsToUse);

        if (detailedResult.productCode == "Н/Д") {
            detailedResult.productCode = findAdditionalProduct(digitsToUse);
        }
    }
    else if (isEAN8(basicResult)) {
        detailedResult.country = findCountryForEAN8(basicResult.digits);
        detailedResult.manufacturerCode = "Нет";
        detailedResult.productCode = findProductForEAN8(basicResult.digits);
    }
    else if (basicResult.type == "UPCE") {
        detailedResult.country = "Специальный формат UPC-E";
        detailedResult.manufacturerCode = "Н/Д";
        detailedResult.productCode = "Н/Д";
    }
    else if (basicResult.type == "CODE128" || basicResult.type == "CODE39") {
        detailedResult.country = "Не применимо";
        detailedResult.manufacturerCode = "Н/Д";
        detailedResult.productCode = "Н/Д";
    }

    // Заполнение значений по умолчанию
    if (detailedResult.country.empty()) detailedResult.country = "Неизвестно";
    if (detailedResult.manufacturerCode.empty()) detailedResult.manufacturerCode = "Н/Д";
    if (detailedResult.productCode.empty()) detailedResult.productCode = "Н/Д";

    std::cout << "Final detailed result - Country: " << detailedResult.country
              << ", Manufacturer: " << detailedResult.manufacturerCode
              << ", Product: " << detailedResult.productCode << std::endl;

    return detailedResult;
}

void BarcodeReader::saveToFile(const BarcodeResult& result) {
    std::string filePath = "C:/Users/rauko/Desktop/data_files/Barcode_All.txt";
    std::ofstream file(filePath, std::ios::app);
    if (!file.is_open()) {
        throw FileException("Не удалось открыть файл для записи: " + filePath);
    }

    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);

    // Простое решение для Windows
    std::tm tmBuf;
    localtime_s(&tmBuf, &time);  // Используем Windows-специфичную функцию

    std::ostringstream oss;
    oss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S");
    std::string timeStr = oss.str();

    file << "=== " << timeStr << " ===\n";
    file << "Тип: " << result.type << "\n";
    file << "Цифры: " << result.digits << "\n";
    file << "Страна: " << result.country << "\n";
    file << "Код производителя: " << result.manufacturerCode << "\n";
    file << "Код товара: " << result.productCode << "\n";
    file << "----------------------------------------\n";
    file.close();
}

