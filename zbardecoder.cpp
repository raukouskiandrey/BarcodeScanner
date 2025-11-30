#include "ZBarDecoder.h"
#include "BarcodeResult.h"
#include <iostream>

ZBarDecoder::ZBarDecoder() {
    zbar_scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);
}

std::string ZBarDecoder::decodeWithZBar(const cv::Mat& roi) {
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

std::string ZBarDecoder::filterBarcodeResult(const std::string& result) {
    if (result.empty()) return "";

    // Разрешаем все типы штрих-кодов, которые может обнаружить ZBar
    std::cout << "ZBar raw result: " << result << std::endl;
    return result;
}

BarcodeResult ZBarDecoder::parseZBarResult(const std::string& zbarResult) {
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
    }
    else {
        // Если формат нестандартный, пытаемся извлечь данные
        result.type = "Unknown Format";
        result.digits = zbarResult;
        result.fullResult = zbarResult;
    }

    return result;
}
