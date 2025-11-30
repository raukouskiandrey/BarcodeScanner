#include "BarcodeReader2D.h"
#include <iostream>
#include <fstream>

BarcodeReader2D::BarcodeReader2D() {}
BarcodeReader2D::~BarcodeReader2D() {}

BarcodeResult BarcodeReader2D::decode(const cv::Mat& image) {
    BarcodeResult result;
    result.type = "2D";
    result.digits = "";

    if (image.empty()) {
        result.type = "Ошибка";
        result.digits = "Пустое изображение";
        return result;
    }

    auto decodedList = opencv2DDetector.detectAndDecode(image);
    if (!decodedList.empty()) {
        result = createDetailedResult(decodedList[0]);
    }

    return result;
}

BarcodeResult BarcodeReader2D::decode(const std::string& filename) {
    cv::Mat image = cv::imread(filename);
    if (image.empty()) {
        BarcodeResult result;
        result.type = "Ошибка";
        result.digits = "Не удалось загрузить изображение";
        return result;
    }
    return decode(image);
}

BarcodeResult BarcodeReader2D::createDetailedResult(const std::string& rawData) {
    BarcodeResult result;
    result.type = "QR/DataMatrix";
    result.digits = rawData;
    result.fullResult = rawData;

    // Здесь можно добавить парсинг: например, если QR содержит URL или JSON
    if (rawData.find("http") == 0) {
        result.productCode = "Ссылка";
    } else {
        result.productCode = "Данные";
    }

    result.country = "Н/Д";
    result.manufacturerCode = "Н/Д";

    std::cout << "Final 2D result: " << rawData << std::endl;
    return result;
}

void BarcodeReader2D::saveToFile(const BarcodeResult& result)
{
    const std::string filename = "C:/Users/rauko/Desktop/Barcode_All.txt";  // фиксированное имя файла

    std::ofstream file(filename, std::ios::app); // открываем в режиме добавления
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл для сохранения: " << filename << std::endl;
        return;
    }

    file << "=== Результат сканирования 2D ===" << std::endl;
    file << "Тип: " << result.type << std::endl;
    file << "Полный код: " << result.digits << std::endl;

    // Для QR/DataMatrix сохраняем только интерпретацию (например, "Ссылка" или "Данные")
    if (!result.productCode.empty() && result.productCode != "Н/Д") {
        file << "Интерпретация: " << result.productCode << std::endl;
    }

    file << std::endl;
    file.close();

    std::cout << "✅ Результат сохранен в файл: " << filename << std::endl;
}
