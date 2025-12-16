#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include "DecodeException.h"
#include "BarcodeException.h"

class BarcodeDetectorOpenCV {
public:
    std::vector<std::vector<cv::Point>> detectWithOpenCV(const cv::Mat& frame) const;
private:
    cv::barcode::BarcodeDetector opencv_detector;
};
