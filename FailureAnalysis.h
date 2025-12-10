#pragma once
#include <string>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>
#include "BarcodeReader.h"

// Тип проблемы
enum class ProblemType {
    IMAGE_QUALITY,
    DECODER_CONFIG,
    BARCODE_TYPE,
    CAMERA_ISSUE,
    LIGHTING,
    FOCUS_BLUR,
    PERSPECTIVE,
    DAMAGED_BARCODE,
    SIZE_RESOLUTION,
    UNKNOWN
};

// Детали проблемы
struct ProblemDetail {
    ProblemType type;
    std::string description;
    std::string cause;
    std::string recommendation;
    double confidence;
    cv::Mat visualization;

    bool operator<(const ProblemDetail& other) const {
        return confidence > other.confidence; // сортировка по уверенности
    }
};

// Полный анализ
struct FailureAnalysis {
    std::vector<ProblemDetail> problems;
    ProblemDetail primaryProblem;
    double overallScore;
    bool isFixable;
    std::string summary;
    std::map<std::string, double> metrics;

    // Методы для форматирования
    std::string toHtmlReport() const;
    std::string toJson() const;
    std::string toPlainText() const;

    // Методы анализа изображения
    static double calculateSharpness(const cv::Mat& gray);
    static double calculateNoiseLevel(const cv::Mat& gray);
};

class BarcodeReader;

// 👇 Дружественная функция объявляется здесь же
FailureAnalysis analyzeDecodingFailure(
    const BarcodeReader& decoder,
    const cv::Mat& failedImage,
    const std::string& expectedResult);
