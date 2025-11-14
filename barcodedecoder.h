#ifndef BARCODEDECODER_H
#define BARCODEDECODER_H

#include <opencv2/opencv.hpp>
#include <zbar.h>
#include <string>

class BarcodeDecoder
{
public:
    BarcodeDecoder();
    ~BarcodeDecoder();

    std::string decodeWithZBar(const cv::Mat& roi);
    std::string smartDecodeWithUnwarp(const cv::Mat& frame, const cv::Rect& rect);

private:
    zbar::ImageScanner zbar_scanner;
};

#endif // BARCODEDECODER_H
