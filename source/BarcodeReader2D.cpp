#include "BarcodeReader2D.h"
#include "ImageLoadException.h"
#include "DecodeException.h"
#include "FileException.h"
#include <iostream>
#include <fstream>

BarcodeReader2D::BarcodeReader2D() = default;
BarcodeReader2D::~BarcodeReader2D() = default;

BarcodeResult BarcodeReader2D::decode(const cv::Mat& image) {
    if (image.empty()) {
        throw DecodeException("Пустое изображение для декодирования (2D)");
    }

    auto decodedList = opencv2DDetector.detectAndDecode(image);
    if (decodedList.empty()) {
        throw DecodeException("Не удалось распознать 2D штрих-код");
    }

    return createDetailedResult(decodedList[0]);
}

BarcodeResult BarcodeReader2D::decode(const std::string& filename) {
    cv::Mat image = cv::imread(filename);
    if (image.empty()) {
        throw ImageLoadException(filename);
    }
    return decode(image);
}

BarcodeResult BarcodeReader2D::createDetailedResult(const std::string& rawData) const{
    BarcodeResult result;
    result.type = "QR/DataMatrix";
    result.digits = rawData;
    result.fullResult = rawData;

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

void BarcodeReader2D::saveToFile(const BarcodeResult& result) {
    const std::string filename = "C:/Users/rauko/Desktop/data_files/Barcode_All.txt";

    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        throw FileException("Не удалось открыть файл для сохранения: " + filename);
    }

    file << "=== Результат сканирования 2D ===" << std::endl;
    file << "Тип: " << result.type << std::endl;
    file << "Полный код: " << result.digits << std::endl;

    if (!result.productCode.empty() && result.productCode != "Н/Д") {
        file << "Интерпретация: " << result.productCode << std::endl;
    }

    file << std::endl;
    file.close();

    std::cout << "✅ Результат сохранен в файл: " << filename << std::endl;
}
