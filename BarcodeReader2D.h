#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include "BarcodeDetectorOpenCV2D.h"
#include "BarcodeResult.h"
#include "AbstractDecoder.h"

class BarcodeReader2D : public AbstractDecoder {
public:
    BarcodeReader2D();
    ~BarcodeReader2D();

    BarcodeResult decode(const cv::Mat& image) override;
    BarcodeResult decode(const std::string& filename) override;
    std::string getDecoderName() const override { return "BarcodeReader2D"; }

    void saveToFile(const BarcodeResult& result) override;

private:
    BarcodeDetectorOpenCV2D opencv2DDetector;
    BarcodeResult createDetailedResult(const std::string& rawData);
};
