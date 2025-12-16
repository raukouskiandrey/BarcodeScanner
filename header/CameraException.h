#pragma once
#include "BarcodeException.h"

class CameraException : public BarcodeException {
public:
    explicit CameraException(const std::string& message)
        : BarcodeException("Ошибка камеры: " + message) {}
};
