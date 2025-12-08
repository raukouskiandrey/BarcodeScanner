#pragma once
#include "BarcodeException.h"

// Ошибка загрузки изображения
class ImageLoadException : public BarcodeException {
public:
    explicit ImageLoadException(const std::string& filename)
        : BarcodeException("Ошибка загрузки изображения: " + filename) {}
};
