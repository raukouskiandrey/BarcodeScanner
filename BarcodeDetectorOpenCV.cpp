#include "BarcodeDetectorOpenCV.h"
#include <iostream>

std::vector<std::vector<cv::Point>> BarcodeDetectorOpenCV::detectWithOpenCV(const cv::Mat& frame) const{
    std::vector<std::vector<cv::Point>> polygons;
    std::vector<cv::Point> corners;
    std::vector<std::string> decoded_info;
    std::vector<std::string> decoded_type;

    try {
        bool detected = opencv_detector.detectAndDecodeWithType(frame, decoded_info, decoded_type, corners);

        if (detected && !corners.empty() && corners.size() % 4 == 0) {
            for (int i = 0; i < (int)corners.size(); i += 4) {
                if (i + 3 < corners.size()) {
                    std::vector<cv::Point> polygon = { corners[i], corners[i + 1], corners[i + 2], corners[i + 3] };
                    polygons.push_back(polygon);
                }
            }
        }
    }
    catch (const cv::Exception& e) {
        std::cerr << "OpenCV error: " << e.what() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
    }


    return polygons;
}


