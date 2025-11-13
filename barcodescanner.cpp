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


// Статические переменные
std::map<std::string, std::string> BarcodeReader::countryCodesMap;
bool BarcodeReader::countryCodesLoaded = false;

// Конструктор
BarcodeReader::BarcodeReader() {
    // Разрешаем все популярные типы штрих-кодов
    zbar_scanner.set_config(zbar::ZBAR_EAN13, zbar::ZBAR_CFG_ENABLE, 1);
    zbar_scanner.set_config(zbar::ZBAR_EAN8, zbar::ZBAR_CFG_ENABLE, 1);
    zbar_scanner.set_config(zbar::ZBAR_UPCA, zbar::ZBAR_CFG_ENABLE, 1);
    zbar_scanner.set_config(zbar::ZBAR_UPCE, zbar::ZBAR_CFG_ENABLE, 1);
    zbar_scanner.set_config(zbar::ZBAR_CODE128, zbar::ZBAR_CFG_ENABLE, 1);
    zbar_scanner.set_config(zbar::ZBAR_CODE39, zbar::ZBAR_CFG_ENABLE, 1);

    // Загружаем коды стран при создании первого экземпляра
    if (!countryCodesLoaded) {
        loadCountryCodes();
    }
}

void BarcodeReader::loadCountryCodes() {
    std::string filePath = "C:/Users/rauko/Desktop/Barcode_Countries.txt";
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл кодов стран: " << filePath << std::endl;
        // Создаем резервную базу на случай ошибки
        countryCodesMap = {
            {"460", "Россия"}, {"461", "Россия"}, {"462", "Россия"}, {"463", "Россия"}, {"464", "Россия"},
            {"465", "Россия"}, {"466", "Россия"}, {"467", "Россия"}, {"468", "Россия"}, {"469", "Россия"},
            {"490", "Япония"}, {"520", "Греция"}, {"570", "Дания"}, {"690", "Китай"}
        };
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string codes = line.substr(0, colon_pos);
            std::string country = line.substr(colon_pos + 1);

            // Обрабатываем диапазоны (например "460-469")
            size_t dash_pos = codes.find('-');
            if (dash_pos != std::string::npos) {
                std::string start_str = codes.substr(0, dash_pos);
                std::string end_str = codes.substr(dash_pos + 1);

                try {
                    int start = std::stoi(start_str);
                    int end = std::stoi(end_str);

                    for (int i = start; i <= end; i++) {
                        std::string code = std::to_string(i);
                        countryCodesMap[code] = country;
                    }
                } catch (...) {
                    std::cerr << "Ошибка парсинга диапазона: " << codes << std::endl;
                }
            } else {
                // Одиночный код
                countryCodesMap[codes] = country;
            }
        }
    }

    file.close();
    countryCodesLoaded = true;
    std::cout << "Загружено кодов стран: " << countryCodesMap.size() << std::endl;
}

std::string BarcodeReader::getCountryName(const std::string& prefix) {
    if (!countryCodesLoaded) {
        loadCountryCodes();
    }

    auto it = countryCodesMap.find(prefix);
    if (it != countryCodesMap.end()) {
        return it->second;
    }

    // Проверяем частичные совпадения для длинных префиксов
    if (prefix.length() > 3) {
        for (int len = prefix.length() - 1; len >= 2; len--) {
            std::string sub_prefix = prefix.substr(0, len);
            it = countryCodesMap.find(sub_prefix);
            if (it != countryCodesMap.end()) {
                return it->second;
            }
        }
    }

    return "Неизвестно";
}

BarcodeReader::BarcodeResult BarcodeReader::createDetailedResult(const BarcodeResult& basicResult) {
    BarcodeResult detailedResult = basicResult;

    // Инициализируем поля, если они пустые
    if (detailedResult.country.empty()) detailedResult.country = "Неизвестно";
    if (detailedResult.manufacturerCode.empty()) detailedResult.manufacturerCode = "Н/Д";
    if (detailedResult.productCode.empty()) detailedResult.productCode = "Н/Д";

    // Нормализуем тип штрих-кода
    std::string normalizedType = basicResult.type;
    if (normalizedType == "EAN13") normalizedType = "EAN-13";
    if (normalizedType == "EAN8") normalizedType = "EAN-8";
    if (normalizedType == "UPC-A") normalizedType = "UPCA";
    if (normalizedType == "UPC-E") normalizedType = "UPCE";

    std::cout << "Creating detailed result for type: " << basicResult.type
              << " (normalized: " << normalizedType << "), digits: " << basicResult.digits << std::endl;

    // Извлекаем информацию о стране, производителе и товаре
    if ((normalizedType == "EAN-13" || normalizedType == "UPCA") && basicResult.digits.length() >= 12) {
        // EAN-13: первые 3 цифры - страна
        // UPC-A: первые цифры также указывают на страну
        std::string digitsToUse = basicResult.digits;

        // Для UPC-A преобразуем в EAN-13 формат (добавляем 0 в начало)
        if (normalizedType == "UPCA" && digitsToUse.length() == 12) {
            digitsToUse = "0" + digitsToUse;
        }

        if (digitsToUse.length() >= 3) {
            std::string country_code = digitsToUse.substr(0, 3);
            detailedResult.country = getCountryName(country_code);
            std::cout << "EAN-13/UPC-A country code: " << country_code
                      << " -> " << detailedResult.country << std::endl;
        }

        if (digitsToUse.length() >= 7) {
            detailedResult.manufacturerCode = digitsToUse.substr(3, 4);
            detailedResult.productCode = digitsToUse.substr(7, 5);
        }
    }
    else if (normalizedType == "EAN-8" && basicResult.digits.length() == 8) {
        // EAN-8: первые 2-3 цифры - страна
        if (basicResult.digits.length() >= 2) {
            // Пробуем сначала 2 цифры
            std::string country_code = basicResult.digits.substr(0, 2);
            detailedResult.country = getCountryName(country_code);

            // Если не нашли, пробуем 3 цифры
            if (detailedResult.country == "Неизвестно" && basicResult.digits.length() >= 3) {
                country_code = basicResult.digits.substr(0, 3);
                detailedResult.country = getCountryName(country_code);
                std::cout << "EAN-8 country code (3-digit): " << country_code
                          << " -> " << detailedResult.country << std::endl;
            } else {
                std::cout << "EAN-8 country code (2-digit): " << country_code
                          << " -> " << detailedResult.country << std::endl;
            }
        }

        // В EAN-8 нет отдельного кода производителя
        detailedResult.manufacturerCode = "Нет";
        if (basicResult.digits.length() >= 5) {
            detailedResult.productCode = basicResult.digits.substr(2, 5);
        } else if (basicResult.digits.length() >= 3) {
            detailedResult.productCode = basicResult.digits.substr(2);
        }
    }
    else if (normalizedType == "UPCE" && basicResult.digits.length() == 8) {
        // UPC-E: требует специальной обработки
        detailedResult.country = "Специальный формат UPC-E";
        detailedResult.manufacturerCode = "Н/Д";
        detailedResult.productCode = "Н/Д";
    }
    else if (normalizedType == "CODE128" || normalizedType == "CODE39") {
        // Для линейных кодов страна обычно не определяется
        detailedResult.country = "Не применимо";
        detailedResult.manufacturerCode = "Н/Д";
        detailedResult.productCode = "Н/Д";
    }
    else {
        // Для неизвестных типов
        std::cout << "Unknown barcode type for detailed analysis: " << normalizedType
                  << " with length: " << basicResult.digits.length() << std::endl;
    }

    std::cout << "Final detailed result - Country: " << detailedResult.country
              << ", Manufacturer: " << detailedResult.manufacturerCode
              << ", Product: " << detailedResult.productCode << std::endl;

    return detailedResult;
}

std::vector<int> BarcodeReader::extractBits(const std::vector<int>& profile) {
    return std::vector<int>();
}

BarcodeReader::BarcodeResult BarcodeReader::decodeBits(const std::vector<int>& bits) {
    BarcodeResult result;
    result.type = "Неизвестно";
    return result;
}

BarcodeReader::BarcodeResult BarcodeReader::tryDecodeEAN13(const std::string& bitstream) {
    BarcodeResult result;
    result.type = "EAN-13";
    return result;
}

BarcodeReader::BarcodeResult BarcodeReader::tryDecodeEAN8(const std::string& bitstream) {
    BarcodeResult result;
    result.type = "EAN-8";
    return result;
}

bool BarcodeReader::validateChecksum(const std::string& ean) {
    return true;
}

BarcodeReader::BarcodeResult BarcodeReader::decode(const cv::Mat& image) {
    return advancedDecode(image);
}

BarcodeReader::BarcodeResult BarcodeReader::decode(const std::string& filename) {
    cv::Mat image = cv::imread(filename);
    if (image.empty()) {
        BarcodeResult result;
        result.type = "Ошибка";
        result.digits = "Не удалось загрузить изображение";
        return result;
    }
    return advancedDecode(image);
}

void BarcodeReader::saveToFile(const BarcodeResult& result) {
    saveToMainFile(result);
    saveToCountryFile(result);
    saveToManufacturerFile(result);
    saveToProductFile(result);
    saveToStatisticsFile(result);
}

void BarcodeReader::saveToFile(const std::string& barcodeData) {
    std::string filePath = "C:\\Users\\rauko\\Desktop\\Barcode_Simple.txt";

    std::ofstream file(filePath, std::ios::app);
    if (file.is_open()) {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);

        std::string timeStr = std::ctime(&time);
        if (!timeStr.empty() && timeStr[timeStr.length()-1] == '\n') {
            timeStr.erase(timeStr.length()-1);
        }

        file << "[" << timeStr << "] Штрих-код: " << barcodeData << std::endl;
        file.close();
    }
}

void BarcodeReader::saveToMainFile(const BarcodeResult& result) {
    std::string filePath = "C:\\Users\\rauko\\Desktop\\Barcode_All.txt";

    std::ofstream file(filePath, std::ios::app);
    if (file.is_open()) {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);

        std::string timeStr = std::ctime(&time);
        if (!timeStr.empty() && timeStr[timeStr.length()-1] == '\n') {
            timeStr.erase(timeStr.length()-1);
        }

        file << "=== " << timeStr << " ===" << std::endl;
        file << "Тип: " << result.type << std::endl;
        file << "Цифры: " << result.digits << std::endl;
        file << "Страна: " << result.country << std::endl;
        file << "Код производителя: " << result.manufacturerCode << std::endl;
        file << "Код товара: " << result.productCode << std::endl;
        file << "----------------------------------------" << std::endl;
        file.close();
    }
}

void BarcodeReader::saveToCountryFile(const BarcodeResult& result) {
    if (result.country.empty() || result.country == "Неизвестно") return;

    std::string filePath = "C:\\Users\\rauko\\Desktop\\Barcode_Countries.txt";

    std::ofstream file(filePath, std::ios::app);
    if (file.is_open()) {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);

        std::string timeStr = std::ctime(&time);
        if (!timeStr.empty() && timeStr[timeStr.length()-1] == '\n') {
            timeStr.erase(timeStr.length()-1);
        }

        file << "[" << timeStr << "] ";
        file << result.type << " " << result.digits;
        file << " -> " << result.country << std::endl;
        file.close();
    }
}

void BarcodeReader::saveToManufacturerFile(const BarcodeResult& result) {
    if (result.manufacturerCode.empty() || result.manufacturerCode == "Н/Д" || result.manufacturerCode == "Нет") return;

    std::string filePath = "C:\\Users\\rauko\\Desktop\\Barcode_Manufacturers.txt";

    std::ofstream file(filePath, std::ios::app);
    if (file.is_open()) {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);

        std::string timeStr = std::ctime(&time);
        if (!timeStr.empty() && timeStr[timeStr.length()-1] == '\n') {
            timeStr.erase(timeStr.length()-1);
        }

        file << "[" << timeStr << "] ";
        file << result.type << " " << result.digits;
        file << " -> Код производителя: " << result.manufacturerCode;
        file << " (Страна: " << result.country << ")" << std::endl;
        file.close();
    }
}

void BarcodeReader::saveToProductFile(const BarcodeResult& result) {
    if (result.productCode.empty() || result.productCode == "Н/Д") return;

    std::string filePath = "C:\\Users\\rauko\\Desktop\\Barcode_Products.txt";

    std::ofstream file(filePath, std::ios::app);
    if (file.is_open()) {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);

        std::string timeStr = std::ctime(&time);
        if (!timeStr.empty() && timeStr[timeStr.length()-1] == '\n') {
            timeStr.erase(timeStr.length()-1);
        }

        file << "[" << timeStr << "] ";
        file << result.type << " " << result.digits;
        file << " -> Код товара: " << result.productCode;
        file << " (Производитель: " << result.manufacturerCode << ")" << std::endl;
        file.close();
    }
}

void BarcodeReader::saveToStatisticsFile(const BarcodeResult& result) {
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
        outFile << "Тип: " << type << " - " << count << " шт." << std::endl;
    }

    outFile.close();
}

// 🔥 ADVANCED BARCODE SCANNER РЕАЛИЗАЦИЯ

BarcodeReader::BarcodeResult BarcodeReader::advancedDecode(const cv::Mat& image) {
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

    // 🔍 1. ОБЫЧНЫЕ ШТРИХ-КОДЫ (OpenCV)
    auto polygons = detectWithOpenCV(frame);
    std::cout << "OpenCV обнаружено полигонов: " << polygons.size() << std::endl;

    for (const auto& polygon : polygons) {
        if (polygon.size() != 4) continue;

        cv::Rect bbox = cv::boundingRect(polygon);
        cv::Mat roi = frame(bbox);
        std::string zbarResult = decodeWithZBar(roi);

        zbarResult = filterBarcodeResult(zbarResult);

        if (!zbarResult.empty()) {
            BarcodeResult parsedResult = parseZBarResult(zbarResult);
            if (parsedResult.type != "Неизвестно") {
                std::cout << "УСПЕХ: Распознан через OpenCV + ZBar" << std::endl;
                return createDetailedResult(parsedResult);
            }
        }
    }

    // 🔥 2. СЛОЖНЫЕ ШТРИХ-КОДЫ (мятые пакеты, бутылки)
    std::vector<cv::Rect> curved_regions = detectCurvedBarcodesOptimized(frame);
    std::cout << "Обнаружено изогнутых регионов: " << curved_regions.size() << std::endl;

    for (const auto& rect : curved_regions) {
        std::string zbarResult = smartDecodeWithUnwarp(frame, rect);

        if (!zbarResult.empty()) {
            BarcodeResult parsedResult = parseZBarResult(zbarResult);
            if (parsedResult.type != "Неизвестно") {
                std::cout << "УСПЕХ: Распознан через curved detection" << std::endl;
                return createDetailedResult(parsedResult);
            }
        }
    }

    // 🔍 3. ПРЯМОЙ СКАН ВСЕГО ИЗОБРАЖЕНИЯ ZBar
    std::cout << "Пытаемся прямой ZBar scan всего изображения..." << std::endl;
    std::string directResult = decodeWithZBar(frame);
    directResult = filterBarcodeResult(directResult);

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

std::vector<std::vector<cv::Point>> BarcodeReader::detectWithOpenCV(const cv::Mat& frame) {
    std::vector<std::vector<cv::Point>> polygons;
    std::vector<cv::Point> corners;
    std::vector<std::string> decoded_info, decoded_type;

    try {
        bool detected = opencv_detector.detectAndDecodeWithType(frame, decoded_info, decoded_type, corners);

        if (detected && !corners.empty() && corners.size() % 4 == 0) {
            for (int i = 0; i < (int)corners.size(); i += 4) {
                if (i + 3 < corners.size()) {
                    std::vector<cv::Point> polygon = { corners[i], corners[i + 1], corners[i + 2], corners[i + 3] };
                    polygons.push_back(polygon);
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "OpenCV detection error: " << e.what() << std::endl;
    }

    return polygons;
}

std::vector<cv::Rect> BarcodeReader::detectCurvedBarcodesOptimized(const cv::Mat& frame) {
    std::vector<cv::Rect> curved_regions;

    cv::Mat small_frame;
    cv::resize(frame, small_frame, cv::Size(320, 240));

    cv::Mat gray;
    cv::cvtColor(small_frame, gray, cv::COLOR_BGR2GRAY);

    std::vector<cv::Mat> binary_images;

    cv::Mat binary1, binary2, binary3;
    cv::adaptiveThreshold(gray, binary1, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                          cv::THRESH_BINARY, 21, 5);
    cv::adaptiveThreshold(gray, binary2, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                          cv::THRESH_BINARY, 31, 10);
    cv::threshold(gray, binary3, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    binary_images.push_back(binary1);
    binary_images.push_back(binary2);
    binary_images.push_back(binary3);

    cv::Mat grad_x, grad_y;
    cv::Sobel(gray, grad_x, CV_16S, 1, 0, 3);
    cv::Sobel(gray, grad_y, CV_16S, 0, 1, 3);
    cv::convertScaleAbs(grad_x, grad_x);
    cv::convertScaleAbs(grad_y, grad_y);
    cv::Mat gradients;
    cv::addWeighted(grad_x, 0.5, grad_y, 0.5, 0, gradients);
    cv::threshold(gradients, gradients, 50, 255, cv::THRESH_BINARY);
    binary_images.push_back(gradients);

    for (const auto& binary : binary_images) {
        std::vector<cv::Rect> contour_regions = extractRegionsFromContours(binary, small_frame.size());
        curved_regions.insert(curved_regions.end(), contour_regions.begin(), contour_regions.end());
    }

    curved_regions = removeDuplicateRegions(curved_regions);

    double scale_x = (double)frame.cols / small_frame.cols;
    double scale_y = (double)frame.rows / small_frame.rows;

    for (auto& bbox : curved_regions) {
        bbox.x = (int)(bbox.x * scale_x);
        bbox.y = (int)(bbox.y * scale_y);
        bbox.width = (int)(bbox.width * scale_x);
        bbox.height = (int)(bbox.height * scale_y);
    }

    return curved_regions;
}

std::vector<cv::Rect> BarcodeReader::extractRegionsFromContours(const cv::Mat& binary, const cv::Size& image_size) {
    std::vector<cv::Rect> regions;

    cv::Mat morph;
    cv::Mat kernel_horizontal = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 1));
    cv::Mat kernel_vertical = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 3));

    cv::morphologyEx(binary, morph, cv::MORPH_CLOSE, kernel_horizontal);
    cv::morphologyEx(morph, morph, cv::MORPH_OPEN, kernel_vertical);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(morph, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours) {
        if (contour.empty()) continue;

        cv::Rect bbox = cv::boundingRect(contour);

        if (isValidBarcodeRegionExtended(bbox, image_size, contour)) {
            if (hasBarcodeTextureAdvanced(binary(bbox))) {
                cv::Rect expanded_bbox = expandBarcodeRegion(bbox, image_size);
                regions.push_back(expanded_bbox);
            }
        }
    }

    return regions;
}

bool BarcodeReader::isValidBarcodeRegionExtended(const cv::Rect& rect, const cv::Size& image_size,
                                                 const std::vector<cv::Point>& contour) {
    if (rect.width < 30 || rect.height < 10) return false;
    if (rect.width > image_size.width * 0.7 || rect.height > image_size.height * 0.7) return false;

    double area = rect.width * rect.height;
    if (area < 500) return false;

    double aspect_ratio = (double)rect.width / rect.height;
    bool valid_aspect = (aspect_ratio > 1.0 && aspect_ratio < 15.0);

    double contour_area = cv::contourArea(contour);
    double extent = contour_area / area;
    bool valid_contour = (extent > 0.3);

    return valid_aspect && valid_contour;
}

cv::Rect BarcodeReader::expandBarcodeRegion(const cv::Rect& original, const cv::Size& image_size) {
    int expand_x = original.width * 0.2;
    int expand_y = original.height * 0.3;

    cv::Rect expanded = original;
    expanded.x = std::max(0, original.x - expand_x / 2);
    expanded.y = std::max(0, original.y - expand_y / 2);
    expanded.width = std::min(image_size.width - expanded.x, original.width + expand_x);
    expanded.height = std::min(image_size.height - expanded.y, original.height + expand_y);

    return expanded;
}

std::vector<cv::Rect> BarcodeReader::removeDuplicateRegions(const std::vector<cv::Rect>& regions) {
    std::vector<cv::Rect> unique_regions;

    for (const auto& rect : regions) {
        bool is_duplicate = false;

        for (const auto& existing : unique_regions) {
            cv::Rect intersection = rect & existing;
            double overlap = (double)intersection.area() / std::min(rect.area(), existing.area());

            if (overlap > 0.6) {
                is_duplicate = true;
                break;
            }
        }

        if (!is_duplicate) {
            unique_regions.push_back(rect);
        }
    }

    return unique_regions;
}

bool BarcodeReader::hasBarcodeTextureAdvanced(const cv::Mat& region) {
    if (region.empty() || region.rows < 5 || region.cols < 5) return false;

    cv::Mat gray = region;
    if (region.channels() == 3) {
        cv::cvtColor(region, gray, cv::COLOR_BGR2GRAY);
    }

    cv::Mat grad_x, grad_y;
    cv::Sobel(gray, grad_x, CV_32F, 1, 0, 3);
    cv::Sobel(gray, grad_y, CV_32F, 0, 1, 3);

    cv::Scalar mean_x, stddev_x, mean_y, stddev_y;
    cv::meanStdDev(grad_x, mean_x, stddev_x);
    cv::meanStdDev(grad_y, mean_y, stddev_y);

    double horizontal_stripe = stddev_x[0] / (std::abs(mean_x[0]) + 1e-5);
    double vertical_stripe = stddev_y[0] / (std::abs(mean_y[0]) + 1e-5);

    bool is_barcode_like = (horizontal_stripe > 1.8) && (horizontal_stripe > vertical_stripe * 1.2);

    return is_barcode_like;
}

std::string BarcodeReader::smartDecodeWithUnwarp(const cv::Mat& frame, const cv::Rect& rect) {
    if (rect.empty()) return "";

    cv::Mat roi = frame(rect).clone();
    std::vector<cv::Mat> processing_options;

    processing_options.push_back(roi);

    if (rect.width < 150) {
        cv::Mat scaled;
        double scale = std::min(3.0, 200.0 / rect.width);
        cv::resize(roi, scaled, cv::Size(), scale, scale, cv::INTER_CUBIC);
        processing_options.push_back(scaled);
    }

    processing_options.push_back(enhanceContrast(roi));
    processing_options.push_back(enhanceSharpness(roi, 2.0));

    for (int i = 0; i < processing_options.size(); i++) {
        std::string result = decodeWithZBar(processing_options[i]);

        if (!result.empty()) {
            std::cout << "Curved barcode decoded with option " << i << ": " << result << std::endl;
            return result;
        }
    }

    return "";
}

cv::Mat BarcodeReader::enhanceContrast(const cv::Mat& input) {
    cv::Mat lab;
    cv::cvtColor(input, lab, cv::COLOR_BGR2Lab);

    std::vector<cv::Mat> lab_planes;
    cv::split(lab, lab_planes);

    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(lab_planes[0], lab_planes[0]);

    cv::merge(lab_planes, lab);
    cv::Mat result;
    cv::cvtColor(lab, result, cv::COLOR_Lab2BGR);

    return result;
}

cv::Mat BarcodeReader::enhanceSharpness(const cv::Mat& input, double strength) {
    cv::Mat blurred, sharpened;
    cv::GaussianBlur(input, blurred, cv::Size(0, 0), 1.0);
    cv::addWeighted(input, 1.0 + strength, blurred, -strength, 0, sharpened);
    return sharpened;
}

std::string BarcodeReader::decodeWithZBar(const cv::Mat& roi) {
    cv::Mat gray;
    if (roi.channels() == 3) {
        cv::cvtColor(roi, gray, cv::COLOR_BGR2GRAY);
    }
    else {
        gray = roi.clone();
    }

    if (gray.cols < 100 || gray.rows < 40) {
        double scale = std::max(150.0 / gray.cols, 60.0 / gray.rows);
        cv::resize(gray, gray, cv::Size(), scale, scale, cv::INTER_CUBIC);
    }

    try {
        zbar::Image zbar_image(gray.cols, gray.rows, "Y800", gray.data, gray.cols * gray.rows);
        int scan_result = zbar_scanner.scan(zbar_image);

        if (scan_result > 0) {
            for (zbar::Image::SymbolIterator symbol = zbar_image.symbol_begin();
                 symbol != zbar_image.symbol_end(); ++symbol) {

                std::string type_name = symbol->get_type_name();
                std::string data = symbol->get_data();

                std::cout << "ZBar detected: " << type_name << " - " << data << std::endl;

                return type_name + ": " + data;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "ZBar error: " << e.what() << std::endl;
    }
    return "";
}

std::string BarcodeReader::filterBarcodeResult(const std::string& result) {
    if (result.empty()) return "";

    // Разрешаем все типы штрих-кодов, которые может обнаружить ZBar
    std::cout << "ZBar raw result: " << result << std::endl;
    return result;
}

BarcodeReader::BarcodeResult BarcodeReader::parseZBarResult(const std::string& zbarResult) {
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
