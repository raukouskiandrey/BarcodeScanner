#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

class CurvedBarcodeDetector {
public:
    std::vector<cv::Rect> detectCurvedBarcodesOptimized(const cv::Mat& frame);

private:
    std::vector<cv::Rect> extractRegionsFromContours(const cv::Mat& binary, const cv::Size& image_size);
    bool isValidBarcodeRegionExtended(const cv::Rect& rect, const cv::Size& image_size, const std::vector<cv::Point>& contour);
    cv::Rect expandBarcodeRegion(const cv::Rect& original, const cv::Size& image_size);
    std::vector<cv::Rect> removeDuplicateRegions(const std::vector<cv::Rect>& regions);
    bool hasBarcodeTextureAdvanced(const cv::Mat& region);
};
