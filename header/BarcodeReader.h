#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include "BarcodeDetectorOpenCV1D.h"
#include "CurvedBarcodeDetector.h"
#include "ImagePreprocessor.h"
#include "ZBarDecoder.h"
#include "SmartDecoder.h"
#include "BarcodeResult.h"
#include "Country.h"
#include "Decoder.h"

class FailureAnalysis;

class BarcodeReader : public AbstractDecoder {
public:
    BarcodeReader();
    ~BarcodeReader() override;
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
    std::string findProduct(const QString& barcode);
    std::string findCountry(std::string_view digits);
    std::string findManufacturer(std::string_view digits);
    std::string findAdditionalProduct(std::string_view digits);
    std::string findCountryForEAN8(std::string_view digits);
    std::string findProductForEAN8(std::string_view digits);
    bool isEAN13orUPCA(const BarcodeResult& result);
    bool isEAN8(const BarcodeResult& result);
    std::string normalizeDigits(const BarcodeResult& result);
    friend FailureAnalysis analyzeDecodingFailure(
        const BarcodeReader& decoder,
        const cv::Mat& failedImage,
        const std::string& expectedResult);
};
