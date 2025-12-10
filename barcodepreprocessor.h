#ifndef BARCODEPREPROCESSOR_H
#define BARCODEPREPROCESSOR_H

#include <opencv2/opencv.hpp>

class BarcodePreprocessor
{
public:
    BarcodePreprocessor();

    cv::Mat enhanceContrast(const cv::Mat& input);
    cv::Mat enhanceSharpness(const cv::Mat& input, double strength);
};

#endif // BARCODEPREPROCESSOR_H
