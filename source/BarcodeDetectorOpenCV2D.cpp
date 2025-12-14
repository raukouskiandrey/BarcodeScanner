#include "BarcodeDetectorOpenCV2D.h"
#include <iostream>

std::vector<std::string> BarcodeDetectorOpenCV2D::detectAndDecode(const cv::Mat& frame) const{
    std::vector<std::string> results;

    try {
        std::vector<cv::Point> points;
        std::string decoded = qrDetector.detectAndDecode(frame, points);

        if (!decoded.empty()) {
            results.push_back(decoded);
            std::cout << "QR/2D detected: " << decoded << std::endl;
        }
    }
    catch (const cv::Exception& e) {
        std::cerr << "OpenCV 2D error: " << e.what() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
    }

    return results;
}
