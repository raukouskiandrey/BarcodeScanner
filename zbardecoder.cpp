#include "zbardecoder.h"
#include <iostream>

ZBarDecoder::ZBarDecoder() {
    zbar_scanner.set_config(zbar::ZBAR_EAN13, zbar::ZBAR_CFG_ENABLE, 1);
    zbar_scanner.set_config(zbar::ZBAR_EAN8, zbar::ZBAR_CFG_ENABLE, 1);
    zbar_scanner.set_config(zbar::ZBAR_UPCA, zbar::ZBAR_CFG_ENABLE, 1);
    zbar_scanner.set_config(zbar::ZBAR_UPCE, zbar::ZBAR_CFG_ENABLE, 1);
    zbar_scanner.set_config(zbar::ZBAR_CODE128, zbar::ZBAR_CFG_ENABLE, 1);
    zbar_scanner.set_config(zbar::ZBAR_CODE39, zbar::ZBAR_CFG_ENABLE, 1);
}

std::string ZBarDecoder::decode(const cv::Mat& roi) {
    cv::Mat gray;
    if (roi.channels() == 3) {
        cv::cvtColor(roi, gray, cv::COLOR_BGR2GRAY);
    } else {
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
    } catch (const std::exception& e) {
        std::cerr << "ZBar error: " << e.what() << std::endl;
    }
    return "";
}

ZBarResult ZBarDecoder::parseResult(const std::string& zbarResult) {
    ZBarResult result;

    if (zbarResult.empty()) {
        return result;
    }

    size_t colon_pos = zbarResult.find(":");
    if (colon_pos != std::string::npos) {
        std::string type = zbarResult.substr(0, colon_pos);
        std::string digits = zbarResult.substr(colon_pos + 2); // +2 чтобы пропустить ": "

        result.type = type;
        result.digits = digits;
        result.fullResult = zbarResult;
    } else {
        result.type = "Unknown Format";
        result.digits = zbarResult;
        result.fullResult = zbarResult;
    }

    return result;
}

std::string ZBarDecoder::filterBarcodeResult(const std::string& result) {
    if (result.empty()) return "";
    std::cout << "ZBar raw result: " << result << std::endl;
    return result;
}
