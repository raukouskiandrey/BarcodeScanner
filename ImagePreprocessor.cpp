#include "ImagePreprocessor.h"

cv::Mat ImagePreprocessor::enhanceContrast(const cv::Mat& input) const {

    cv::Mat lab;
    cv::cvtColor(input, lab, cv::COLOR_BGR2Lab);

    std::vector<cv::Mat> lab_planes;
    cv::split(lab, lab_planes);

    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(lab_planes[0], lab_planes[0]);

    cv::merge(lab_planes, lab);
    cv::Mat result;
    cv::cvtColor(lab, result, cv::COLOR_Lab2BGR);

    return result;
}



cv::Mat ImagePreprocessor::enhanceSharpness(const cv::Mat& input, double strength) const{

    cv::Mat blurred;
    cv::Mat sharpened;
    cv::GaussianBlur(input, blurred, cv::Size(0, 0), 1.0);
    cv::addWeighted(input, 1.0 + strength, blurred, -strength, 0, sharpened);
    return sharpened;
}


