#include "SmartDecoder.h"
#include "ZBarDecoder.h"
#include <iostream>

SmartDecoder::SmartDecoder(ImagePreprocessor& p, ZBarDecoder& d)
    : preprocessor(p), decoder(d) {
    // Дополнительная инициализация, если нужна
    std::cout << "SmartDecoder initialized" << std::endl;
}

std::string SmartDecoder::smartDecodeWithUnwarp(const cv::Mat& frame, const cv::Rect& rect) {
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

    processing_options.push_back(preprocessor.enhanceContrast(roi));
    processing_options.push_back(preprocessor.enhanceSharpness(roi, 2.0));

    // УДАЛИТЕ эту строку - она лишняя и содержит ошибку:
    // std::string result = decoder.decodeWithZBar(processing_options[i]);

    for (int i = 0; i < processing_options.size(); i++) {
        std::string result = decoder.decodeWithZBar(processing_options[i]); // Используйте decoder.

        if (!result.empty()) {
            std::cout << "Curved barcode decoded with option " << i << ": " << result << std::endl;
            return result;
        }
    }

    return "";
}
// УДАЛИТЕ лишнюю закрывающую скобку в конце файла
