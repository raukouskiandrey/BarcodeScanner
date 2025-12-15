#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <vector>
#include <string>
#include "DecodeException.h"
#include "BarcodeException.h"
class BarcodeDetectorOpenCV2D {
public:
    std::vector<std::string> detectAndDecode(const cv::Mat& frame) const;
private:
    cv::QRCodeDetector qrDetector;
};
