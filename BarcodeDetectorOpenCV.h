#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

class BarcodeDetectorOpenCV {
public:
    std::vector<std::vector<cv::Point>> detectWithOpenCV(const cv::Mat& frame);
private:
    cv::barcode::BarcodeDetector opencv_detector;
};
