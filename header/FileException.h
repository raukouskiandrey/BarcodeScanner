#pragma once
#include "BarcodeException.h"

// Ошибка работы с файлом (сохранение/чтение)
class FileException : public BarcodeException {
public:
    explicit FileException(const std::string& message)
        : BarcodeException("Ошибка файла: " + message) {}
};
