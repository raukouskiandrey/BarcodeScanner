#pragma once
#include <opencv2/opencv.hpp>

class ImagePreprocessor {
public:
    cv::Mat enhanceContrast(const cv::Mat& input) const;
    cv::Mat enhanceSharpness(const cv::Mat& input, double strength) const;
};
