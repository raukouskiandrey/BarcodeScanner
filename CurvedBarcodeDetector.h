#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

class CurvedBarcodeDetector {
public:
    std::vector<cv::Rect> detectCurvedBarcodesOptimized(const cv::Mat& frame);

private:
    std::vector<cv::Rect> extractRegionsFromContours(const cv::Mat& binary, const cv::Size& image_size) const;
    bool isValidBarcodeRegionExtended(const cv::Rect& rect, const cv::Size& image_size, const std::vector<cv::Point>& contour) const;
    cv::Rect expandBarcodeRegion(const cv::Rect& original, const cv::Size& image_size) const;
    std::vector<cv::Rect> removeDuplicateRegions(const std::vector<cv::Rect>& regions) const;
    bool hasBarcodeTextureAdvanced(const cv::Mat& region) const;
};
