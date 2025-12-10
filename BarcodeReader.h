#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include "BarcodeDetectorOpenCV.h"
#include "CurvedBarcodeDetector.h"
#include "ImagePreprocessor.h"
#include "ZBarDecoder.h"
#include "SmartDecoder.h"
#include "BarcodeResult.h"
#include "Country.h"
#include "AbstractDecoder.h"

// Вперед объявление вместо полного включения
class FailureAnalysis;

class BarcodeReader : public AbstractDecoder {
public:
    BarcodeReader();
    ~BarcodeReader() override;  // Добавлен override

    BarcodeResult decode(const cv::Mat& image) override;
    BarcodeResult decode(const std::string& filename) override;
    std::string getDecoderName() const override { return "BarcodeReader"; }

    BarcodeResult advancedDecode(const cv::Mat& image);
    BarcodeResult createDetailedResult(const BarcodeResult& basicResult);
    void saveToFile(const BarcodeResult& result) override;

private:
    [[no_unique_address]] BarcodeDetectorOpenCV opencvDetector;
    [[no_unique_address]] CurvedBarcodeDetector curvedDetector;
    [[no_unique_address]] ImagePreprocessor preprocessor;
    [[no_unique_address]] ZBarDecoder zbarDecoder;
    [[no_unique_address]] SmartDecoder smartDecoder;

    std::vector<cv::Rect> detectCurvedBarcodesOptimized(const cv::Mat& image);
    std::string filterBarcodeResult(const std::string& result);
    BarcodeResult parseZBarResult(const std::string& zbarResult);
    std::string smartDecodeWithUnwarp(const cv::Mat& frame, const cv::Rect& rect);

    // ❌ УДАЛИ это (дубликат, уже есть выше)
    // class FailureAnalysis;

    // ✅ Корректное объявление friend функции
    friend FailureAnalysis analyzeDecodingFailure(
        const BarcodeReader& decoder,
        const cv::Mat& failedImage,
        const std::string& expectedResult);
};
