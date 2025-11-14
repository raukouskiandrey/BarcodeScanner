#ifndef OPENCVDETECTOR_H
#define OPENCVDETECTOR_H

#include <opencv2/opencv.hpp>
#include <vector>

class OpenCVDetector {
public:
    OpenCVDetector();
    std::vector<std::vector<cv::Point>> detect(const cv::Mat& frame);
};

#endif // OPENCVDETECTOR_H
