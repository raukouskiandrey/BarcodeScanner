#pragma once
#include "BarcodeException.h"

// Ошибка декодирования штрих-кода
class DecodeException : public BarcodeException {
public:
    explicit DecodeException(const std::string& message)
        : BarcodeException("Ошибка декодирования: " + message) {}
};
