#ifndef BARCODEREGIONFINDER_H
#define BARCODEREGIONFINDER_H

#include <opencv2/opencv.hpp>
#include <vector>

class BarcodeRegionFinder
{
public:
    BarcodeRegionFinder();

    std::vector<cv::Rect> findBarcodeRegions(const cv::Mat& image);

private:
    bool isValidROI(const cv::Rect& roi, const cv::Size& imageSize);
    cv::Rect addPadding(const cv::Rect& original, const cv::Size& imageSize, int padding);
    std::vector<cv::Rect> removeDuplicateRegions(const std::vector<cv::Rect>& regions);
};

#endif // BARCODEREGIONFINDER_H
