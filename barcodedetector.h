#ifndef BARCODEDETECTOR_H
#define BARCODEDETECTOR_H

#include <opencv2/opencv.hpp>
#include <vector>

class BarcodeDetector
{
public:
    BarcodeDetector();

    std::vector<std::vector<cv::Point>> detectWithOpenCV(const cv::Mat& frame);
    std::vector<cv::Rect> detectCurvedBarcodesOptimized(const cv::Mat& frame);

private:
    cv::barcode::BarcodeDetector opencv_detector;
};

#endif // BARCODEDETECTOR_H
