#ifndef SMARTDECODER_H
#define SMARTDECODER_H

#include "ImagePreprocessor.h"
#include "ZBarDecoder.h"
#include <opencv2/opencv.hpp>
#include <string>

class SmartDecoder {
public:
    SmartDecoder(ImagePreprocessor& preprocessor, ZBarDecoder& decoder);
    std::string smartDecodeWithUnwarp(const cv::Mat& frame, const cv::Rect& rect);

private:
    ImagePreprocessor& preprocessor;
    ZBarDecoder& decoder;
};

#endif // SMARTDECODER_H
