#pragma once
#include <string>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>
#include "BarcodeReader.h"
#include <compare>

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

    // Конструктор по умолчанию
    ProblemDetail() noexcept
        : type(ProblemType::UNKNOWN), confidence(0.0) {}

    // Конструктор с параметрами
    ProblemDetail(ProblemType t, std::string desc, std::string c,
                  std::string rec, double conf, cv::Mat viz = cv::Mat()) noexcept
        : type(t), description(std::move(desc)), cause(std::move(c)),
        recommendation(std::move(rec)), confidence(conf), visualization(std::move(viz)) {}

    // Конструктор копирования
    ProblemDetail(const ProblemDetail& other) = default;

    // Оператор присваивания копированием
    ProblemDetail& operator=(const ProblemDetail& other) = default;

    // Конструктор перемещения (noexcept)
    ProblemDetail(ProblemDetail&& other) noexcept
        : type(other.type), description(std::move(other.description)),
        cause(std::move(other.cause)), recommendation(std::move(other.recommendation)),
        confidence(other.confidence), visualization(std::move(other.visualization)) {}

    // Оператор присваивания перемещением (noexcept)
    ProblemDetail& operator=(ProblemDetail&& other) noexcept {
        if (this != &other) {
            type = other.type;
            description = std::move(other.description);
            cause = std::move(other.cause);
            recommendation = std::move(other.recommendation);
            confidence = other.confidence;
            visualization = std::move(other.visualization);
        }
        return *this;
    }

    // Деструктор
    ~ProblemDetail() = default;

    auto operator<=>(const ProblemDetail& other) const = default;

};

// Полный анализ
struct FailureAnalysis {
    std::vector<ProblemDetail> problems;
    ProblemDetail primaryProblem;
    double overallScore;
    bool isFixable;
    std::string summary;
    std::map<std::string, double> metrics;

    // Конструктор по умолчанию
    FailureAnalysis() noexcept
        : overallScore(0.0), isFixable(false) {}

    // Конструктор копирования
    FailureAnalysis(const FailureAnalysis& other) = default;

    // Конструктор перемещения (noexcept)
    FailureAnalysis(FailureAnalysis&& other) noexcept
        : problems(std::move(other.problems)),
        primaryProblem(std::move(other.primaryProblem)),
        overallScore(other.overallScore),
        isFixable(other.isFixable),
        summary(std::move(other.summary)),
        metrics(std::move(other.metrics)) {}

    // Оператор присваивания копированием
    FailureAnalysis& operator=(const FailureAnalysis& other) = default;

    // Оператор присваивания перемещением (noexcept)
    FailureAnalysis& operator=(FailureAnalysis&& other) noexcept {
        if (this != &other) {
            problems = std::move(other.problems);
            primaryProblem = std::move(other.primaryProblem);
            overallScore = other.overallScore;
            isFixable = other.isFixable;
            summary = std::move(other.summary);
            metrics = std::move(other.metrics);
        }
        return *this;
    }

    // Деструктор
    ~FailureAnalysis() = default;

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
