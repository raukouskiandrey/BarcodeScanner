#pragma once
#include <opencv2/opencv.hpp>
#include <zbar.h>
#include <string>
#include "BarcodeResult.h"
#include "DecodeException.h"
#include "BarcodeException.h"

class ZBarDecoder {
private:
    zbar::ImageScanner zbar_scanner;
public:
    ZBarDecoder(); // Явное объявление конструктора
    ~ZBarDecoder() = default; // И деструктора тоже

    std::string decodeWithZBar(const cv::Mat& roi);
    std::string filterBarcodeResult(const std::string& result);
    BarcodeResult parseZBarResult(std::string_view zbarResult);
};
