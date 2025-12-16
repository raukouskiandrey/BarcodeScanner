#pragma once
#include <string>
#include <opencv2/opencv.hpp>
#include "BarcodeResult.h"

class AbstractDecoder {
public:
    virtual ~AbstractDecoder() = default;

    virtual BarcodeResult decode(const cv::Mat& image) = 0;
    virtual BarcodeResult decode(const std::string& filename) = 0;
    virtual std::string getDecoderName() const = 0;
    virtual bool canSaveToFile() const { return true; }
    virtual void saveToFile(const BarcodeResult& result) = 0;
};
