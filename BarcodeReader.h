#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include "BarcodeDetectorOpenCV.h"
#include "CurvedBarcodeDetector.h"
#include "ImagePreprocessor.h"
#include "ZBarDecoder.h"
#include "SmartDecoder.h"
#include "BarcodeResult.h"
#include "Country.h"
#include "AbstractDecoder.h"

class BarcodeReader : public AbstractDecoder {
public:
    BarcodeReader();
    ~BarcodeReader();

    BarcodeResult decode(const cv::Mat& image) override;
    BarcodeResult decode(const std::string& filename) override;
    std::string getDecoderName() const override { return "BarcodeReader"; }

    BarcodeResult advancedDecode(const cv::Mat& image);
    BarcodeResult createDetailedResult(const BarcodeResult& basicResult);
    void saveToFile(const BarcodeResult& result) override;

private:
    BarcodeDetectorOpenCV opencvDetector;
    CurvedBarcodeDetector curvedDetector;
    ImagePreprocessor preprocessor;
    ZBarDecoder zbarDecoder;
    SmartDecoder smartDecoder;

    std::vector<cv::Rect> detectCurvedBarcodesOptimized(const cv::Mat& image);
    std::string filterBarcodeResult(const std::string& result);
    BarcodeResult parseZBarResult(const std::string& zbarResult);
    std::string smartDecodeWithUnwarp(const cv::Mat& frame, const cv::Rect& rect);
};
