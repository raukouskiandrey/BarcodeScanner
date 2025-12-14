#pragma once
#include "BarcodeException.h"

// Ошибка работы камеры
class CameraException : public BarcodeException {
public:
    explicit CameraException(const std::string& message)
        : BarcodeException("Ошибка камеры: " + message) {}
};
