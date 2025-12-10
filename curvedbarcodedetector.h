#ifndef CURVEDBARCODEDETECTOR_H
#define CURVEDBARCODEDETECTOR_H

#include <opencv2/opencv.hpp>
#include <vector>

class CurvedBarcodeDetector {
public:
    CurvedBarcodeDetector();
    std::vector<cv::Rect> detectCurvedBarcodes(const cv::Mat& frame);
};

#endif // CURVEDBARCODEDETECTOR_H
