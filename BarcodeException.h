#pragma once
#include <stdexcept>
#include <string>

class BarcodeException : public std::runtime_error {
public:
    explicit BarcodeException(const std::string& message)
        : std::runtime_error("BarcodeException: " + message) {}
};
