#ifndef ABSTRACTDECODER_H
#define ABSTRACTDECODER_H

#include <string>
#include <opencv2/opencv.hpp>
#include "BarcodeResult.h"

class AbstractDecoder {
public:
    virtual ~AbstractDecoder() = default;

    // Чисто виртуальные функции (делают класс абстрактным)
    virtual BarcodeResult decode(const cv::Mat& image) = 0;
    virtual BarcodeResult decode(const std::string& filename) = 0;
    virtual std::string getDecoderName() const = 0;

    // Виртуальная функция с реализацией по умолчанию
    virtual bool canSaveToFile() const { return true; }

    // Виртуальная функция для сохранения
    virtual void saveToFile(const BarcodeResult& result) = 0;
};

#endif // ABSTRACTDECODER_H
