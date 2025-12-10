#ifndef ZBARDECODER_H
#define ZBARDECODER_H

#include <opencv2/opencv.hpp>
#include <zbar.h>
#include <string>

// Переименуем структуру чтобы избежать конфликта
struct ZBarResult {
    std::string type;
    std::string digits;
    std::string fullResult;
    std::string country;
    std::string manufacturerCode;
    std::string productCode;

    ZBarResult()
        : type("Неизвестно"), digits(""), fullResult(""),
        country(""), manufacturerCode(""), productCode("") {}
};

class ZBarDecoder
{
private:
    zbar::ImageScanner zbar_scanner;

public:
    ZBarDecoder();
    ~ZBarDecoder() = default;

    std::string decode(const cv::Mat& roi);
    ZBarResult parseResult(const std::string& zbarResult);

private:
    std::string filterBarcodeResult(const std::string& result);
};

#endif // ZBARDECODER_H
