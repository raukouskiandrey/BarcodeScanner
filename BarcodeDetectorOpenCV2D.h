#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <vector>
#include <string>

class BarcodeDetectorOpenCV2D {
public:
    std::vector<std::string> detectAndDecode(const cv::Mat& frame);
private:
    cv::QRCodeDetector qrDetector;
};
