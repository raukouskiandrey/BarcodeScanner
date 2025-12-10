#include "BarcodeDetectorOpenCV2D.h"
#include <iostream>

std::vector<std::string> BarcodeDetectorOpenCV2D::detectAndDecode(const cv::Mat& frame) {
    std::vector<std::string> results;

    try {
        std::vector<cv::Point> points;
        std::string decoded = qrDetector.detectAndDecode(frame, points);

        if (!decoded.empty()) {
            results.push_back(decoded);
            std::cout << "QR/2D detected: " << decoded << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "OpenCV 2D detection error: " << e.what() << std::endl;
    }

    return results;
}
