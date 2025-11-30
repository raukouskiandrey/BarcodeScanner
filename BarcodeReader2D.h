#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include "BarcodeDetectorOpenCV2D.h"
#include "BarcodeResult.h"

class BarcodeReader2D {
public:
    BarcodeReader2D();
    ~BarcodeReader2D();

    BarcodeResult decode(const cv::Mat& image);
    BarcodeResult decode(const std::string& filename);

    void saveToFile(const BarcodeResult& result);

private:
    BarcodeDetectorOpenCV2D opencv2DDetector;
    BarcodeResult createDetailedResult(const std::string& rawData);
};
