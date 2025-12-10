#include "barcodedecoder.h"
#include <iostream>
#include <opencv2/opencv.hpp>

BarcodeDecoder::BarcodeDecoder()
{
    zbar_scanner.set_config(zbar::ZBAR_EAN13, zbar::ZBAR_CFG_ENABLE, 1);
    zbar_scanner.set_config(zbar::ZBAR_EAN8, zbar::ZBAR_CFG_ENABLE, 1);
    zbar_scanner.set_config(zbar::ZBAR_UPCA, zbar::ZBAR_CFG_ENABLE, 1);
    zbar_scanner.set_config(zbar::ZBAR_UPCE, zbar::ZBAR_CFG_ENABLE, 1);
    zbar_scanner.set_config(zbar::ZBAR_CODE128, zbar::ZBAR_CFG_ENABLE, 1);
    zbar_scanner.set_config(zbar::ZBAR_CODE39, zbar::ZBAR_CFG_ENABLE, 1);
}

BarcodeDecoder::~BarcodeDecoder()
{
}

std::string BarcodeDecoder::decodeWithZBar(const cv::Mat& roi)
{
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

std::string BarcodeDecoder::smartDecodeWithUnwarp(const cv::Mat& frame, const cv::Rect& rect)
{
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

    // Здесь будет вызов BarcodePreprocessor
    // processing_options.push_back(enhanceContrast(roi));
    // processing_options.push_back(enhanceSharpness(roi, 2.0));

    for (int i = 0; i < processing_options.size(); i++) {
        std::string result = decodeWithZBar(processing_options[i]);

        if (!result.empty()) {
            std::cout << "Curved barcode decoded with option " << i << ": " << result << std::endl;
            return result;
        }
    }

    return "";
}
